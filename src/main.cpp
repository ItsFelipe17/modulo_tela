/*
Autores: ItsFelipe17, flavio-as, judetur-gif, kauacordeirodev, tchindjiarufina.
Programa: Tela automatizada
Descrição: Controle de tela automatizada por Radiofrequência
Data: 08/06/2026
Versão: 2.0
*/

#include <Arduino.h>
#include "TelaProjecaoRF.h"
#include <ArduinoJson.h>
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "DebugManager.h"
#include <Preferences.h>

const uint8_t PINO_TX = 7; // Emissor
const uint8_t PINO_RX = 6; // Received

// ===== TÓPICO MQTT =====
const char TOPICO_COMANDO[] = "senai134/shared/projeto/emissor";

// ===== PROTÓTIPOS ===== mqtt
void tratarMensagemRecebida(const char *, const String &);
void tratarJsonComando(const String &);
void enviarHandshakeTela();
void obterCodigoControle();
void enviarComando(uint8_t endereco[5], uint8_t comando);

TelaProjecaoRF telaRF(PINO_TX, PINO_RX);
Preferences preferences;

uint8_t enderecoTela1[5] = {0};
uint8_t enderecoTela2[5] = {0};

bool statusHandshake = false;
bool modoAprendizado = false;
uint8_t telaAprendizado = 0;
uint8_t comando;

void salvarEndereco(const char *chave, const uint8_t endereco[5])
{
  // Abre o namespace "telas" para leitura e escrita
  preferences.begin("telas", false);

  // Salva os 5 bytes do endereço usando a chave informada
  // Exemplo: chave = "tela1"
  preferences.putBytes(chave, endereco, 5);
  preferences.end();
}

bool carregarEndereco(const char *chave, uint8_t endereco[5])
{
  preferences.begin("telas", true);
  // Verifica se existe um valor salvo para essa chave
  // e se ele possui exatamente 5 bytes
  if (preferences.getBytesLength(chave) != 5)
  {
    preferences.end();
    // Retorna false indicando que não encontrou um endereço válido
    return false;
  }

  preferences.getBytes(chave, endereco, 5);
  preferences.end();
  // Retorna true indicando que encontrou um endereço válido
  return true;
}
void setup()
{
  conectarWiFi();
  configurarMQTT();
  configurarDebug();
  conectarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  debugInfo("Sistema pronto");
  Serial.begin(9600);
  telaRF.begin(&Serial);         // &Serial permite que a biblioteca escreva mensagens no monitor serial.
  telaRF.setInverterSinal(true); // Ajuste conforme seu hardware
  carregarEndereco("tela1", enderecoTela1);
  carregarEndereco("tela2", enderecoTela2);
  if (carregarEndereco("tela1", enderecoTela1))
  {
    debugInfo("Tela 1 carregada.");
  }

  if (carregarEndereco("tela2", enderecoTela2))
  {
    debugInfo("Tela 2 carregada.");
  }
}

void loop()
{
  telaRF.update(); // Obrigatório
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();
  obterCodigoControle();
}

void obterCodigoControle()
{
  if (!modoAprendizado || !telaRF.enderecoCapturadoDisponivel())
    return;

  uint8_t endereco[5];

  if (!telaRF.obterEnderecoCapturado(endereco))
    return;

  uint8_t *destino = nullptr;
  const char *chave = nullptr;

  switch (telaAprendizado)
  {
  case 1:
    destino = enderecoTela1;
    chave = "tela1";
    break;

  case 2:
    destino = enderecoTela2;
    chave = "tela2";
    break;

  default:
    return;
  }

  memcpy(destino, endereco, 5);
  salvarEndereco(chave, destino);

  debugInfo("Endereco salvo.");

  modoAprendizado = false;
  telaAprendizado = 0;
}

void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
  if (strcmp(topico, TOPICO_COMANDO) != 0)
  {
    return;
  }

  tratarJsonComando(mensagem);
}

void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;

  DeserializationError erro = deserializeJson(doc, mensagem);

  if (erro)
  {
    debugErro("Erro na estrutura JSON.");
    debugErro(erro.c_str());
    return;
  }

  statusHandshake = true;

  //* @details
  //* Numeraçao dos comandos
  //* Deve se adcionar o número da tela e depois o comando
  //* Se nunca usado é necessário aprender a tela
  //* Comandos
  //* Subir -> 0 | Descer -> 1 | Parar -> 2 | Aprender -> 3

  if (doc["tela_1"])
  {
    comando = doc["tela_1"]["comando"] | 0;

    if (comando == 3)
    {
      telaAprendizado = 1;
      modoAprendizado = true;
      telaRF.iniciarLeituraEndereco();
      return;
    }

    enviarComando(enderecoTela1, comando);
  }

  if (doc["tela_2"])
  {
    comando = doc["tela_2"]["comando"] | 0;

    if (comando == 3)
    {
      telaAprendizado = 2;
      modoAprendizado = true;
      telaRF.iniciarLeituraEndereco();
      return;
    }

    enviarComando(enderecoTela2, comando);
  }

  if (doc["telas"])
  {
    comando = doc["telas"]["comando"] | 0;

    if (comando == 3)
    {
      debugErro("Aprendizado deve ser feito por tela.");
      return;
    }

    enviarComando(enderecoTela1, comando);
    enviarComando(enderecoTela2, comando);
  }

}

void enviarComando(uint8_t endereco[5], uint8_t comando)
{
  switch (comando)
  {
  case 0:
    telaRF.enviarCima(endereco);
    enviarHandshakeTela();
    break;

  case 1:
    telaRF.enviarBaixo(endereco);
    enviarHandshakeTela();
    break;

  case 2:
    telaRF.enviarParar(endereco);
    enviarHandshakeTela();
    break;

  default:
    debugErro("Comando invalido.");
    break;
  }
}

void enviarHandshakeTela()
{
  if (!statusHandshake)
    return;

  if (statusHandshake)
  {
    JsonDocument doc;
    String mensagem;

    doc["statusComando"]["comando"] = comando;
    doc["statusComando"]["situacao"] = statusHandshake;

    serializeJson(doc, mensagem);
    publicarMensagemNoTopico(0, mensagem.c_str());
  }

  statusHandshake = false;
}

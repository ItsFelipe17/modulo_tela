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

TelaProjecaoRF telaRF(PINO_TX, PINO_RX);
Preferences preferences;

uint8_t enderecoTela1[5] = {0};
uint8_t enderecoTela2[5] = {0};

bool modoAprendizado = false;
uint8_t telaAprendizado = 0;

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
    telaRF.begin(&Serial);      // &Serial permite que a biblioteca escreva mensagens no monitor serial.
    telaRF.setInverterSinal(true); // Ajuste conforme seu hardware
    carregarEndereco("tela1", enderecoTela1);
    carregarEndereco("tela2", enderecoTela2);
    debugInfo("Enderecos carregados.");
}

void loop()
{
    garantirWiFiConectado();
    garantirMQTTConectado();
    loopMQTT();
    telaRF.update(); // Obrigatório
    if (modoAprendizado &&
        telaRF.enderecoCapturadoDisponivel())
    {
        uint8_t endereco[5];

        if (telaRF.obterEnderecoCapturado(endereco))
        {
            if (telaAprendizado == 1)
            {
                memcpy(enderecoTela1, endereco, 5);
                salvarEndereco("tela1", enderecoTela1);

                debugInfo("Endereco da Tela 1 salvo.");
            }
            else if (telaAprendizado == 2)
            {
                memcpy(enderecoTela2, endereco, 5);
                salvarEndereco("tela2", enderecoTela2);

                debugInfo("Endereco da Tela 2 salvo.");
            }

            modoAprendizado = false;
            telaAprendizado = 0;
        }
    }
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
    //* @details
//* Numeraçao dos comandos
//* Deve se adcionar o número da tela e depois o comando
//* Se nunca usado é necessário aprender a tela 
//* Comandos
//* Subir -> 0 | Descer -> 1 | Parar -> 2 | Aprender -> 3
uint8_t tela = 0;
uint8_t comando = 0;

if (doc["tela_1"])
{
    tela = 1;
    comando = doc["tela_1"]["comando"] | 0;
}
else if (doc["tela_2"])
{
    tela = 2;
    comando = doc["tela_2"]["comando"] | 0;
}
else if (doc["telas"])
{
    tela = 3;
    comando = doc["telas"]["comando"] | 0;
}


Serial.print("Tela: ");
Serial.println(tela);

Serial.print("Comando: ");
Serial.println(comando);

if (comando == 3)
{
    telaAprendizado = tela;
    modoAprendizado = true;

    telaRF.iniciarLeituraEndereco();

    debugInfo("Pressione um botao do controle da tela.");
    return;
}

if (tela == 1)
{
    if (comando == 0)
        telaRF.enviarCima(enderecoTela1);
    else if (comando == 1)
        telaRF.enviarBaixo(enderecoTela1);
    else if (comando == 2)
        telaRF.enviarParar(enderecoTela1);
}
else if (tela == 2)
{
    if (comando == 0)
        telaRF.enviarCima(enderecoTela2);
    else if (comando == 1)
        telaRF.enviarBaixo(enderecoTela2);
    else if (comando == 2)
        telaRF.enviarParar(enderecoTela2);
}
else if (tela == 3)
{
    if (comando == 0)
        telaRF.enviarCima(enderecoTela1);
    else if (comando == 1)
        telaRF.enviarBaixo(enderecoTela1);
    else if (comando == 2)
        telaRF.enviarParar(enderecoTela1);

    if (comando == 0)
        telaRF.enviarCima(enderecoTela2);
    else if (comando == 1)
        telaRF.enviarBaixo(enderecoTela2);
    else if (comando == 2)
        telaRF.enviarParar(enderecoTela2);
}
}
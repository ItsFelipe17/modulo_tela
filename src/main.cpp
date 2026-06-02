#include <Arduino.h>
#include "TelaProjecaoRF.h"
#include <ArduinoJson.h>
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "DebugManager.h"
#include <Preferences.h>

const uint8_t PINO_TX = 7;
const uint8_t PINO_RX = 6;

// ===== TÓPICO MQTT =====
const char TOPICO_COMANDO[] = "senai134/equipe/mario/devices/tela";

// ===== PROTÓTIPOS ===== mqtt
void tratarMensagemRecebida(const char *, const String &);
void tratarJsonComando(const String &);

TelaProjecaoRF telaRF(PINO_TX, PINO_RX);
Preferences preferences;

// Insira aqui o endereco capturado no exemplo anterior
const uint8_t ENDERECO_DA_MINHA_TELA1[5] = {0xCD, 0x4E, 0x0A, 0x01, 0x00};  //Tela do lado Direito
const uint8_t ENDERECO_DA_MINHA_TELA2[5] = {0xCD, 0x4B, 0xF6, 0x01, 0x00}; //Tela do lado esquerdo

void setup()
{
  conectarWiFi();
  configurarMQTT();
  configurarDebug();
  conectarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  debugInfo("Sistema pronto");
  Serial.begin(9600);
  telaRF.begin(&Serial);         // o que está entre parênteses é o argumento que está sendo passado para a função
  telaRF.setInverterSinal(true); // Ajuste conforme seu hardware
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();
  telaRF.update(); // Obrigatório
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
//* Iremos trabalhar com dois digítos -> 00 
//* Subir -> 11 | Descer -> 12 | Parar -> 13

//* Formato Json para mensagem {
//*  "tela": 1,
//*  "comando": 11
//*}


  uint8_t tela = 0;
  uint8_t comando = 0;

if (doc["tela"].is<uint8_t>())
{
    tela = doc["tela"].as<uint8_t>();
}

if (doc["comando"].is<uint8_t>())
{
    comando = doc["comando"].as<uint8_t>();
}

//* Formato Json para mensagem {
//*  "tela": 1,
//*  "comando": 11
//*}


    if (tela == 1)
{
    if (comando == 11)
    {
        telaRF.enviarCima(ENDERECO_DA_MINHA_TELA1);
    }
    else if (comando == 12)
    {
        telaRF.enviarBaixo(ENDERECO_DA_MINHA_TELA1);
    }
    else if (comando == 13)
    {
        telaRF.enviarParar(ENDERECO_DA_MINHA_TELA1);
    }
}
else if (tela == 2)
{
    if (comando == 11)
    {
        telaRF.enviarCima(ENDERECO_DA_MINHA_TELA2);
    }
    else if (comando == 12)
    {
        telaRF.enviarBaixo(ENDERECO_DA_MINHA_TELA2);
    }
    else if (comando == 13)
    {
        telaRF.enviarParar(ENDERECO_DA_MINHA_TELA2);
    }
}
else debugErro("Comando inválido");
}
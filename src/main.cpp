#include <Arduino.h>
#include "TelaProjecaoRF.h"
#include <ArduinoJson.h>
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "DebugManager.h"

const uint8_t PINO_TX = 7;
const uint8_t PINO_RX = 6;

// ===== TÓPICO MQTT =====
const char TOPICO_COMANDO[] = "senai134/esp32/projetor";

// ===== PROTÓTIPOS ===== mqtt
void tratarMensagemRecebida(const char *, const String &);
void tratarJsonComando(const String &);


TelaProjecaoRF telaRF(PINO_TX, PINO_RX);

// Insira aqui o endereco capturado no exemplo anterior
const uint8_t ENDERECO_DA_MINHA_TELA[5] = {0xCD, 0x4E, 0x0A, 0x01, 0x00};

void setup()
{
  conectarWiFi();
  configurarMQTT();
  configurarDebug();
  conectarMQTT();
  registrarCallbackMensagem(tratarMensagemRecebida);
  debugInfo("Sistema pronto");
    Serial.begin(9600);
    telaRF.begin(&Serial); //o que está entre parênteses é o argumento que está sendo passado para a função
    telaRF.setInverterSinal(true); // Ajuste conforme seu hardware
    
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();
  telaRF.update(); //Obrigatório 
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

  String comando = "";

  if(doc["comando"].is<String>()) comando = doc["comando"].as<String>();

  if (comando == "subir")
  {
    telaRF.enviarCima(ENDERECO_DA_MINHA_TELA);
  }
  else if (comando == "descer")
  {
    telaRF.enviarBaixo(ENDERECO_DA_MINHA_TELA);
  }
  else if (comando == "parar")
  {
    telaRF.enviarParar(ENDERECO_DA_MINHA_TELA);
  }
  else
  {
    debugErro("Comando inválido");
  }



}
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
const char TOPICO_COMANDO[] = "senai134/equipe/mario/devices/qualquer";

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
    preferences.begin("telas", false);
    preferences.putBytes(chave, endereco, 5);
    preferences.end();
}

bool carregarEndereco(const char *chave, uint8_t endereco[5])
{
    preferences.begin("telas", true);

    if (preferences.getBytesLength(chave) != 5)
    {
        preferences.end();
        return false;
    }

    preferences.getBytes(chave, endereco, 5);
    preferences.end();

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
    telaRF.begin(&Serial);         // o que está entre parênteses é o argumento que está sendo passado para a função
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

    uint8_t tela = doc["tela"] | 0;
    uint8_t comando = doc["comando"] | 0;

    Serial.print("Tela: ");
    Serial.println(tela);

    Serial.print("Comando: ");
    Serial.println(comando);

    if (comando == 1)
    {
        telaAprendizado = tela;
        modoAprendizado = true;

        telaRF.iniciarLeituraEndereco();

        debugInfo("Pressione um botao do controle da tela.");
        return;
    }
        if (enderecoTela1 || enderecoTela2)
            debugInfo("Endereço salvo");
    

    if (tela == 1)
    {
        if (comando == 11)
            telaRF.enviarCima(enderecoTela1);
        else if (comando == 12)
            telaRF.enviarBaixo(enderecoTela1);
        else if (comando == 13)
            telaRF.enviarParar(enderecoTela1);
    }
    else if (tela == 2)
    {
        if (comando == 11)
            telaRF.enviarCima(enderecoTela2);
        else if (comando == 12)
            telaRF.enviarBaixo(enderecoTela2);
        else if (comando == 13)
            telaRF.enviarParar(enderecoTela2);
    }
}
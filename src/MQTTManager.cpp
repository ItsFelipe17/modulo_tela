// * MQTTManager.cpp

#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino.h>

#include "secrets.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "DebugManager.h"

WiFiClient wifiClient;
WiFiClientSecure wifiClientSecure;

PubSubClient mqttClient;

CallbackMensagemMQTT callbackDaAplicacao = nullptr;

void registrarCallbackMensagem(CallbackMensagemMQTT callback)
{
    callbackDaAplicacao = callback;

    if (callbackDaAplicacao != nullptr)
    {
        debugInfo("Callback da aplicação registrado com sucesso.");
    }
    else
    {
        debugErro("Callback da aplicação não foi registrado.");
    }
}

const char *obterTopicoPublicado(int indiceTopico)
{
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_PUBLICAR)
    {
        debugErro("Índice invalido para tópico de publicação " + String(indiceTopico));
        return "";
    }
    return TOPICOS_PUBLICAR[indiceTopico];
}

const char *obterTopicoRecebimento(int indiceTopico)
{
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_RECEBER)
    {
        debugErro("Índice invalido para tópico de recebimecnto " + String(indiceTopico));
        return "";
    }
    return TOPICOS_RECEBER[indiceTopico];
}

void callbackInternoMQTT(char *topico, byte *payload, unsigned long tamanho)
{
    String mensagem = "";

    for (uint8_t i = 0; i < tamanho; i++)
    {
        mensagem += (char)payload[i];
    }

    debugInfoSemLinha("========================\n\r");
    debugInfoSemLinha(" MENSAGEM MQTT RECEBIDA \n\r");
    debugInfoSemLinha("========================\n\r");
    debugInfoSemLinha("TÓPICO: " + String(topico) + "\n\r");
    debugInfo("MENSAGEM: " + mensagem);

    if (callbackDaAplicacao != nullptr)
        callbackDaAplicacao(topico, mensagem);

    else
        debugErro("Mensagem recebida, mas nenhum callback da aplicação foi registrado.");
}

void configurarMQTT()
{
    debugInfoSemLinha("======================\n\r");
    debugInfoSemLinha(" CONFIGURANDO MQTT... \n\r");
    debugInfoSemLinha("======================\n\r");

    if (USAR_AWS_IOT)
    {
        debugInfo("Modo selecionado: AWS IoT Core");

        wifiClientSecure.setCACert(AWS_CERT_CA);
        wifiClientSecure.setCertificate(AWS_CERT_CRT);
        wifiClientSecure.setPrivateKey(AWS_CERT_PRIVATE);

        mqttClient.setClient(wifiClientSecure);
        mqttClient.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORTA);

        debugInfo("Endpoint AWS IoT: " + String(AWS_IOT_ENDPOINT));
        debugInfo("Porta AWS IoT: " + String(AWS_IOT_PORTA));
    }
    else if (MQTT_TLS)
    {
        debugInfo(" MODO SELECIONADO: MQTT COM TLS.");

        if (strlen(MQTT_CERTIFICADO_CA) > 100)
        {
            debugInfo("Certificado CA do broker MQTT configurado.");
            wifiClientSecure.setCACert(MQTT_CERTIFICADO_CA);
        }
        else
        {
            debugErro("Certificado não configurado. Usando setInsecure apenas para teste.");
            wifiClientSecure.setInsecure();
        }

        mqttClient.setClient(wifiClientSecure);
        mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);

        debugInfo(" Broker MQTT: " + String(MQTT_BROKER));
        debugInfo(" Porta MQTT: " + String(MQTT_PORTA));
    }
    else // Conectar ao broker público sem certificado
    {
        debugInfo(" MODO SELECIONADO: MQTT SEM TLS.");

        mqttClient.setClient(wifiClient);
        mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);

        debugInfo(" Broker MQTT: " + String(MQTT_BROKER));
        debugInfo(" Porta MQTT: " + String(MQTT_PORTA));
    }

    mqttClient.setCallback(callbackInternoMQTT);
    debugInfo("Callback interno no MQTT configurado.");
}

void conectarMQTT()
{
    if (!WiFiEstaConectado())
    {
        debugErro("MQTT não pode se conectar porque o WiFI está desconectado.");
        return;
    }

    debugInfoSemLinha("============================\n\r");
    debugInfo(" Iniciando conexão MQTT... ");
    debugInfoSemLinha("============================\n\r");

    uint8_t tentativasMQTT = 0;
    const uint8_t maxTentativasMQTT = 5;

    while (!mqttClient.connected() && tentativasMQTT < maxTentativasMQTT)
    {
        debugInfo("Tentando conectar ao broker MQTT. Tentativa: " + String(tentativasMQTT));

        bool conectado = false;

        if (USAR_AWS_IOT)
        {
            conectado = mqttClient.connect(AWS_IOT_CLIENT_ID);
        }
        else
        {
            if (strlen(MQTT_USUARIO) > 0)
            {
                debugInfo("Conectando MQTT com usuário e senha.");
                conectado = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USUARIO, MQTT_SENHA);
            }
            else // * Conexão em modo anônimo
            {
                debugInfo("Conectando MQTT sem usuário e senha.");
                conectado = mqttClient.connect(MQTT_CLIENT_ID);
            }
        }
        if (conectado)
        {
            debugInfo("MQTT conectado com sucesso.");

            int totalTopicos = obterTotalTopicosRecebimento();

            debugInfo("Total de tópicos para inscrição: " + String(totalTopicos));

            for (uint8_t i = 0; i < totalTopicos; i++)
            {
                const char *topico = obterTopicoRecebimento(i);
                bool inscrito = mqttClient.subscribe(topico);

                if (inscrito)
                {
                    debugInfo("Inscrito no tópico: " + String(topico));
                }
                else
                {
                    debugErro("Falha ao se increver no tópico: " + String(topico));
                }
            }

            publicarMensagemNoTopico(0, "ESP32 conectado ao MQTT.");
        }
        else
        {
            debugErro("Falha ao conectar no MQTT. Código de erro: " + String(mqttClient.state()));
            tentativasMQTT++;
            delay(2000);
        }
    }
    if (!mqttClient.connected())
    {
        debugErro("Não foi possivel conectar ao broker MQTT após " + String(maxTentativasMQTT) + " tentativas.");
    }
}

uint8_t obterTotalTopicosRecebimento()
{
    return TOTAL_TOPICOS_RECEBER;
}

void garantirMQTTConectado()
{
    if (!WiFiEstaConectado)
    {
        debugErro("MQTT não reconectado porque o WiFi está desconectado.");
        return;
    }

    if (!mqttClient.connected())
    {
        debugErro("MQTT desconectado. Tentando reconectar...");
        conectarMQTT();
    }
}

void loopMQTT()
{
    mqttClient.loop();
}

void publicarMensagem(const char *topico, const char *mensagem)
{
    if (!mqttClient.connected())
    {
        debugErro("Não foi possível publicar. MQTT desconectado.");
        return;
    }

    bool publicado = mqttClient.publish(topico, mensagem);

    if (publicado)
    {
        debugInfo("Mensagem publicada via MQTT.");
        debugInfo("Topico: " + String(topico));
        debugInfo("Mensagem: " + String(mensagem));
    }
    else
    {
        debugErro("Falha ao publicar mensagem no tópico: " + String(topico));
    }
}

void publicarMensagemNoTopico(int indiceTopico, const char *mensagem)
{
    const char *topico = obterTopicoPublicado(indiceTopico);

    if (strlen(topico) == 0)
    {
        debugErro("Não foi posível publicar. Índice do tópico inválido: " + String(indiceTopico));
        return;
    }

    publicarMensagem(topico, mensagem);
}

bool mqttEstaConectado()
{
    return mqttClient.connected();
}
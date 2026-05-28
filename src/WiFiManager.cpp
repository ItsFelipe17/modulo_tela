// * WiFIManager.cpp

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "WiFiManager.h"
#include "DebugManager.h"
#include "secrets.h"

bool WiFiEstaConectado()
{
    return WiFi.status() == WL_CONNECTED;
}

void conectarWiFi()
{
    debugInfo("=========================");
    debugInfo("Iniciando conexão WiFi...");
    debugInfo("=========================");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_SENHA);

    debugInfoSemLinha("[INFO] Conectando.");

    uint8_t tentativas = 0;
    const uint8_t maxTentativas = 30;

    while (WiFi.status() != WL_CONNECTED && tentativas < maxTentativas)
    {
        delay(500);
        debugInfoSemLinha(".");
        tentativas++;
    }

    debugInfoSemLinha("\n\r");

    if (WiFi.status() == WL_CONNECTED)
    {
        debugInfo("WiFi conectado com sucesso!");
        debugInfoSemLinha("[INFO] Endereço IP: ");
        debugInfoSemLinha(WiFi.localIP().toString());
        debugInfoSemLinha("\n\r");
    }
    else
    {
        debugErro("Falha ao conectar ao WiFi!");
        debugErro("Verifique o SSID, senha e sinal de rede.");
    }
}

void garantirWiFiConectado()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        debugInfo("WiFI desconectado! Tentando reconectar...");
        conectarWiFi();
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        debugErro("Não foi possível reconectar ao WiFi.");
    }
}
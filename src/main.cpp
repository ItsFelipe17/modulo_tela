#include <Arduino.h>
#include <ArduinoJson.h>
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "DebugManager.h"

// ===== PINO RF =====
constexpr uint8_t TX_PIN = 4;

// ===== TÓPICO MQTT =====
const char TOPICO_COMANDO[] =
"senai134/kauac/esp32/display";

// ===== RF =====
const char* HEADER =
"101000111100110101001110000010100000000100000000";

const char* CMD_PARAR =
"00100011010010011";

const char* CMD_SUBIR =
"00001011001100011";

const char* CMD_DESCER =
"01000011011010011";

const char* FRAME_EXTRA =
"00100100010010101";

// ===== PROTÓTIPOS =====
void tratarMensagemRecebida(
    const char *topico,
    const String &mensagem
);

void bit0();
void bit1();
void enviarBits(const char* bits);
void enviarPacote(const char* comando);

void subir();
void descer();
void parar();

// =====================================================

void setup()
{
    configurarDebug();

    pinMode(TX_PIN, OUTPUT);

    conectarWiFi();

    configurarMQTT();

    registrarCallbackMensagem(tratarMensagemRecebida);

    conectarMQTT();

    debugInfo("Sistema pronto");
}

void loop()
{
    garantirWiFiConectado();

    garantirMQTTConectado();

    loopMQTT();
}

// =====================================================

void tratarMensagemRecebida(
    const char *topico,
    const String &mensagem
)
{
    debugInfo("Mensagem recebida:");
    debugInfo(mensagem);

    if (strcmp(topico, TOPICO_COMANDO) != 0)
    {
        return;
    }

    if (mensagem == "subir")
    {
        subir();
    }
    else if (mensagem == "descer")
    {
        descer();
    }
    else if (mensagem == "parar")
    {
        parar();
    }
    else
    {
        debugErro("Comando inválido");
    }
}

// =====================================================

void bit0()
{
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(350);

    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(1050);
}

void bit1()
{
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(1050);

    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(350);
}

void enviarBits(const char* bits)
{
    while (*bits)
    {
        if (*bits == '1')
        {
            bit1();
        }
        else
        {
            bit0();
        }

        bits++;
    }

    digitalWrite(TX_PIN, LOW);
}

void enviarPacote(const char* comando)
{
    String pacote =
        String(HEADER) + comando;

    enviarBits(pacote.c_str());
}

// =====================================================

void parar()
{
    enviarPacote(CMD_PARAR);

    Serial.println("PARAR");
}

void subir()
{
    enviarPacote(CMD_SUBIR);

    delay(10);

    enviarPacote(FRAME_EXTRA);

    Serial.println("SUBIR");
}

void descer()
{
    enviarPacote(CMD_DESCER);

    delay(10);

    enviarPacote(FRAME_EXTRA);

    Serial.println("DESCER");
}
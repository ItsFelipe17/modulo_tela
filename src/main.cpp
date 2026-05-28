#include <Arduino.h>
#define TX_PIN 4
#define BOTAO_BOOT 0

// =====================================
// HEADER FIXO
// =====================================

const char* HEADER =
"101000111100110101001110000010100000000100000000";

// =====================================
// COMANDOS
// =====================================

const char* CMD_PARAR =
"00100011010010011";

const char* CMD_SUBIR =
"00001011001100011";

const char* CMD_DESCER =
"01000011011010011";

// =====================================
// FRAME EXTRA
// =====================================

const char* FRAME_EXTRA =
"00100100010010101";

// =====================================
// BIT 0
// =====================================

void bit0()
{
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(350);

    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(1050);
}

// =====================================
// BIT 1
// =====================================

void bit1()
{
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(1050);

    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(350);
}

// =====================================
// ENVIA BITS
// =====================================

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

// =====================================
// ENVIA PACOTE COMPLETO
// =====================================

void enviarPacote(const char* comando)
{
    String pacote = String(HEADER) + comando;

    enviarBits(pacote.c_str());
}

// =====================================
// PARAR
// =====================================

void parar()
{
    enviarPacote(CMD_PARAR);

    Serial.println("PARAR");
}

// =====================================
// SUBIR
// =====================================

void subir()
{
    enviarPacote(CMD_SUBIR);

    delay(10);

    enviarPacote(FRAME_EXTRA);

    Serial.println("SUBIR");
}

// =====================================
// DESCER
// =====================================

void descer()
{
    enviarPacote(CMD_DESCER);

    delay(10);

    enviarPacote(FRAME_EXTRA);

    Serial.println("DESCER");
}

// =====================================
// SETUP
// =====================================

void setup()
{
    Serial.begin(115200);

    pinMode(TX_PIN, OUTPUT);

    // botão BOOT
    pinMode(BOTAO_BOOT, INPUT_PULLUP);

    Serial.println("Pronto");
}

// =====================================
// LOOP
// =====================================

void loop()
{
    // botão apertado
    if (digitalRead(BOTAO_BOOT) == LOW)
    {
        subir();

        // debounce
        delay(500);
    }
}


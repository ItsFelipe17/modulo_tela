// * MQTTManager.h

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>

void configurarMQTT();
void conectarMQTT();
void garantirMQTTConectado();
void loopMQTT();

void publicarMensagem(const char *topico, const char *mensagem);
void publicarMensagemNoTopico(int indiceTopico, const char *mensagem);
bool MQTTEstaConectado();

const char* obterTopicoPublicado(int indiceTopico);
const char* obterTopicoRecebimento(int indiceTopico);
uint8_t obterTotalTopicosRecebimento();

typedef void (*CallbackMensagemMQTT)(const char *topico, const String &mensagem);

void registrarCallbackMensagem(CallbackMensagemMQTT callback);

#endif
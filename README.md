# Módulo Tela Automatizada (ESP32/MQTT)

Biblioteca/aplicação Arduino/ESP32 para controle remoto de telas de projeção motorizadas via **MQTT + Radiofrequência 433 MHz**, permitindo integração com sistemas de automação, dashboards e controladores centralizados.

> Autores: ItsFelipe17, flavio-as, judetur-gif, kauacordeirodev, tchindjiarufina
>
> Depende de:
>
> * TelaProjecaoRF
> * PubSubClient
> * ArduinoJson
> * Preferences

---

## Visão geral

O módulo conecta-se a uma rede Wi-Fi e a um broker MQTT.

Ao receber mensagens JSON em um tópico MQTT configurado, converte os comandos recebidos em sinais RF compatíveis com telas de projeção motorizadas.

Também possui um sistema de aprendizado capaz de capturar e armazenar o endereço RF do controle original de cada tela.

Fluxo básico:

```text
Sistema de Automação
          │
          ▼
      MQTT Broker
          │
          ▼
        ESP32
          │
          ▼
 Biblioteca TelaProjecaoRF
          │
          ▼
   Transmissor RF 433 MHz
          │
          ▼
 Tela de Projeção Motorizada
```

---

## Funcionalidades

* ✅ Controle de telas motorizadas por RF 433 MHz.
* ✅ Integração MQTT.
* ✅ Recebimento de comandos via JSON.
* ✅ Aprendizado automático de controles remotos.
* ✅ Armazenamento permanente dos endereços RF.
* ✅ Controle individual de múltiplas telas.
* ✅ Controle simultâneo de todas as telas cadastradas.
* ✅ Reconexão automática de Wi-Fi.
* ✅ Reconexão automática de MQTT.
* ✅ Estrutura modularizada.
* ✅ Sistema de logs para diagnóstico.

---

## Instalação

### PlatformIO

```ini
lib_deps =
    bblanchon/ArduinoJson
    knolleary/PubSubClient
```

Além das dependências:

* TelaProjecaoRF
* Preferences
* WiFi (ESP32)

---

## Estrutura do projeto

```text
src/
├── main.cpp
├── WiFiManager.cpp
├── MQTTManager.cpp
├── DebugManager.cpp

include/
├── WiFiManager.h
├── MQTTManager.h
├── DebugManager.h
├── secrets.h
```

---

## Configuração

As credenciais e parâmetros são definidos em:

```cpp
secrets.h
```

### Wi-Fi

```cpp
const char WIFI_SSID[]  = "MinhaRede";
const char WIFI_SENHA[] = "MinhaSenha";
```

### MQTT

```cpp
const char MQTT_BROKER[] = "broker.exemplo.com";
const uint16_t MQTT_PORTA = 1883;

const char MQTT_CLIENT_ID[] = "esp32_tela";
```

### Tópicos

```cpp
senai134/shared/projeto/emissor
```

---

## Inicialização

O sistema é iniciado em:

```cpp
void setup()
{
    conectarWiFi();

    configurarMQTT();

    configurarDebug();

    registrarCallbackMensagem(
        tratarMensagemRecebida
    );

    conectarMQTT();

    telaRF.begin(&Serial);

    telaRF.setInverterSinal(true);

    carregarEndereco("tela1", enderecoTela1);

    carregarEndereco("tela2", enderecoTela2);
}
```

Loop principal:

```cpp
void loop()
{
    garantirWiFiConectado();

    garantirMQTTConectado();

    loopMQTT();

    telaRF.update();
}
```

---

## Formato das mensagens MQTT

### Comando para Tela 1

```json
{
    "tela_1": {
        "comando": 0
    }
}
```

### Comando para Tela 2

```json
{
    "tela_2": {
        "comando": 1
    }
}
```

### Comando para todas as telas

```json
{
    "telas": {
        "comando": 2
    }
}
```

---

## Tópico de comando

```cpp
senai134/shared/projeto/emissor
```

Toda mensagem recebida nesse tópico é processada por:

```cpp
tratarJsonComando()
```

---

## Comandos suportados

| Índice | Comando              |
| ------ | -------------------- |
| 0      | Subir                |
| 1      | Descer               |
| 2      | Parar                |
| 3      | Aprender endereço RF |

---

## Sistema de aprendizado

O módulo permite cadastrar automaticamente uma tela através do controle remoto original.

### Aprender Tela 1

```json
{
    "tela_1": {
        "comando": 3
    }
}
```

### Aprender Tela 2

```json
{
    "tela_2": {
        "comando": 3
    }
}
```

Após receber o comando, o sistema exibirá:

```text
Pressione um botão do controle da tela.
```

Ao capturar o endereço:

```text
Endereco da Tela 1 salvo.
```

ou

```text
Endereco da Tela 2 salvo.
```

---

## Exemplo de publicação MQTT

### Subir Tela 1

```json
{
    "tela_1": {
        "comando": 0
    }
}
```

### Descer Tela 2

```json
{
    "tela_2": {
        "comando": 1
    }
}
```

### Parar todas as telas

```json
{
    "telas": {
        "comando": 2
    }
}
```

---

## Sistema de MQTT

### Conexão

```cpp
conectarMQTT();
```

### Reconexão automática

```cpp
garantirMQTTConectado();
```

### Recebimento de mensagens

```cpp
registrarCallbackMensagem(
    tratarMensagemRecebida
);
```

---

## Sistema de Wi-Fi

### Conectar

```cpp
conectarWiFi();
```

### Verificar conexão

```cpp
WiFiEstaConectado();
```

### Reconexão automática

```cpp
garantirWiFiConectado();
```

---

## Logs

O sistema utiliza o módulo DebugManager.

Exemplo:

```text
[INFO] Sistema pronto
[INFO] Enderecos carregados
[INFO] Mensagem MQTT recebida
```

Aprendizado:

```text
[INFO] Pressione um botao do controle da tela
[INFO] Endereco da Tela 1 salvo
```

Erros:

```text
[ERRO] Erro na estrutura JSON
[ERRO] MQTT desconectado
```

---

## Fluxo de processamento

```text
MQTT
 │
 ▼
Mensagem JSON
 │
 ▼
ArduinoJson
 │
 ▼
Identificação da Tela
 │
 ▼
Identificação do Comando
 │
 ▼
TelaProjecaoRF
 │
 ▼
Transmissão RF
 │
 ▼
Tela Motorizada
```

---

## Exemplo completo

Publicação MQTT:

```json
{
    "tela_1": {
        "comando": 0
    }
}
```

Processamento:

```cpp
telaRF.enviarCima(
    enderecoTela1
);
```

Resultado:

```text
A Tela 1 sobe.
```

---

## Versão

```text
Versão: 2.0
Data: 08/06/2026
```
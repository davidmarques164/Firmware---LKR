#include "Wifi/wifi.h"
#include "ModbusTcp/modbusTcp.h"
#include "Log/log.h"
#include "NTP/ntp.h"

#include <ArduinoJson.h>
#include "MQTT/mqtt_client.h"   // usa o cliente centralizado

extern Log       lg;
extern ModbusClp clp;
extern ReadTime  ntp;

WiFiManager wm;

bool logOn = 0;

/* Watchdog de Wi-Fi */
static uint8_t  wifiRetryCount = 0;
static const uint8_t WIFI_MAX_RETRIES_BEFORE_AP = 6;

Wifi::Wifi(): macWifi("00.00.00.00"){}

void Wifi::begin(){
    // Políticas de reconexão/timeouts
    wm.setWiFiAutoReconnect(true);
    WiFi.setAutoReconnect(true);
    wm.setConnectTimeout(30);
    wm.setConfigPortalTimeout(180);

    if (wm.autoConnect(SSID)) {
        Serial.println("[Wifi]     - Conectado :)");
        digitalWrite(LED_NET, 1);
        macWifi = WiFi.macAddress();
        Serial.println("[Wifi]     - MAC: " + macWifi);
        logOn = 1;

        // Inicializa/conecta MQTT (usa HOST/PORT/USER/PASS do mqtt_config.h)
        MqttClient::instance().begin(macWifi);
    } else {
        Serial.println("[Wifi]     - Falha ao se conectar");
        lg.salvarLog("WIFI       - Falha ao se conectar.");
        digitalWrite(LED_NET, 0);
        logOn = 0;
    }
}

/* Publica JSON via MQTT */
void Wifi::enviaJson(unsigned long id){
    if (WiFi.status() != WL_CONNECTED) return;

    // Monta JSON (mesma estrutura)
    StaticJsonDocument<1024> jsonDoc;
    jsonDoc["id"] = id;
    jsonDoc["mac"] = macWifi;
    jsonDoc["date_time"] = ntp.formattedDateTime();
    jsonDoc["pallets_produzidos"] = clp.pallets_produzidos;
    jsonDoc["pallets_produzidos_total"] = clp.pallets_produzidos_total;
    jsonDoc["tabuas_empilhadas_1"] = clp.tabuas_empilhadas_1;
    jsonDoc["tabuas_empilhadas_2"] = clp.tabuas_empilhadas_2;
    jsonDoc["tempo_parada_ciclo_entrada"] = clp.tempo_parada_ciclo_entrada;
    jsonDoc["tempo_parada_ciclo_cuber"] = clp.tempo_parada_ciclo_cuber;
    jsonDoc["tempo_parada_ciclo_saida"] = clp.tempo_parada_ciclo_saida;
    jsonDoc["entrada_1"] = digitalRead(S_01);
    jsonDoc["entrada_2"] = digitalRead(S_02);
    jsonDoc["entrada_3"] = digitalRead(S_03);
    jsonDoc["entrada_4"] = digitalRead(S_04);
    jsonDoc["entrada_5"] = digitalRead(S_05);
    jsonDoc["entrada_6"] = digitalRead(S_06);
    jsonDoc["entrada_7"] = digitalRead(S_07);

    String payload;
    serializeJson(jsonDoc, payload);

    // Publica no tópico fixo que você está usando
    const char* TOPICO_FIXO = "/lkr";
    if (MqttClient::instance().publish(String(TOPICO_FIXO), payload)) {
        Serial.println("[Wifi]     - MQTT publish OK");
        for (int i = 0; i < 3; i++){ digitalWrite(LED_SEND,1); delay(50); digitalWrite(LED_SEND,0); delay(400); }
    } else {
        Serial.println("[Wifi]     - MQTT publish FAIL");
        lg.salvarLog("WIFI       - Falha ao publicar MQTT.");
    }
}

void Wifi::wifiLoop(){
    if (WiFi.status() == WL_CONNECTED){
        if (!logOn){
            digitalWrite(LED_NET, 1);
            Serial.println("[Wifi]     - Wifi reconectado!");
            logOn = 1;
        }
        wifiRetryCount = 0;
    } else {
        if (logOn){
            digitalWrite(LED_NET, 0);
            Serial.println("[Wifi]     - Wifi Desconectado");
            Serial.println("[Wifi]     - Tentando reconectar...");
            lg.salvarLog("WIFI       - Wifi desconectado.");
            logOn = 0;
        }
        WiFi.reconnect();
        if (++wifiRetryCount >= WIFI_MAX_RETRIES_BEFORE_AP){
            wifiRetryCount = 0;
            Serial.println("[Wifi]     - Abrindo portal de configuração (AP)...");
            bool ok = wm.startConfigPortal(SSID);
            Serial.println(ok ? "[Wifi]     - Configurado via portal." :
                                 "[Wifi]     - Portal terminou sem conectar.");
        }
        wm.setWiFiAutoReconnect(true);
    }
}

void Wifi::mqttLoop(){
    // Mantém sessão do cliente central
    MqttClient::instance().loop();
}

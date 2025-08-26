#include "Wifi/wifi.h"
#include "ModbusTcp/modbusTcp.h"
#include "Log/log.h"
#include "NTP/ntp.h"

#include <WiFiClientSecure.h>   // (NOVO) HTTPS
// #include <HTTPClient.h>         // (NOVO) http.begin(client, url)
#include <PubSubClient.h>

extern              Log             lg;
extern              ModbusClp       clp;
extern              ReadTime        ntp;

WiFiManager         wm;

bool logOn = 0;

/* Watchdog de Wi-Fi (NOVO) */
static uint8_t wifiRetryCount = 0;
static const uint8_t WIFI_MAX_RETRIES_BEFORE_AP = 6; // ~1 min se TIME_VERIFY_WIFI_S=10

/*
// Endereço do servidor que vai receber os dados  
//const char* server = "https://eduardo-d28ce.firebaseio.com/preMax.json";
const char* server = "https://premax-test-default-rtdb.firebaseio.com/preMax.json"; //base teste
*/

/* ===== MQTT + TLS =====
   Broker público p/ testes (EMQX):
   host: broker.emqx.io  porta TLS: 8883  CA: broker.emqx.io-ca.crt
*/
static const char*   MQTT_HOST  = "broker.emqx.io";
static const uint16_t MQTT_PORT = 1883;            // veja nota TLS abaixo
static const char*   MQTT_USER  = "premax";
static const char*   MQTT_PASS  = "aut0r3aliz3";
static String        MQTT_TOPIC = "premax/lkr";

static WiFiClientSecure mqttNet;   // TLS sempre
static PubSubClient mqtt(mqttNet);

/* CA do broker (COLE o PEM real aqui). Exemplo de placeholder: 
static const char ROOT_CA_PEM[] PROGMEM = R"PEM(
-----BEGIN CERTIFICATE-----
...COLE_AQUI_O_CERTIFICADO_DA_CA_DO_BROKER...
-----END CERTIFICATE-----
)PEM";*/

/* Conecta MQTT com LWT */
static bool mqttConnectWithLWT(const String& clientId){
  String willTopic = "premax/status/" + clientId;
  const uint8_t willQos = 1; const bool willRet = true; const char* willMsg = "offline";
  if (MQTT_USER && MQTT_PASS) {
    return mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS,
                        willTopic.c_str(), willQos, willRet, willMsg);
  } else {
    return mqtt.connect(clientId.c_str(),
                        willTopic.c_str(), willQos, willRet, willMsg);
  }
}

Wifi::Wifi(): macWifi("00.00.00.00"){}
void Wifi::begin(){
    /* (NOVO) Políticas de reconexão e timeouts do WiFiManager */
    wm.setWiFiAutoReconnect(true);   // reconectar automaticamente STA
    WiFi.setAutoReconnect(true);     // reforça no core
    wm.setConnectTimeout(30);        // tenta STA por até 30s antes de desistir
    wm.setConfigPortalTimeout(180);  // se abrir AP, portal fecha após 3 min sem ação

    if(wm.autoConnect(SSID)){
        Serial.println("[Wifi]     - Conectado :)");
        digitalWrite(LED_NET, 1);
        macWifi = WiFi.macAddress();
        Serial.println("[Wifi]     - MAC: " + macWifi);
        //lg.salvarLog("WIFI       - Conectado.");
        logOn = 1;

            /* MQTT TLS */
       //  mqttNet.setCACert(ROOT_CA_PEM);   // valida broker (NTP precisa estar OK)
        mqtt.setServer(MQTT_HOST, MQTT_PORT);
        mqtt.setKeepAlive(30);            // default 15s; margem maior
        mqtt.setBufferSize(1024);         // JSON > 256B

        //MQTT_TOPIC = "premax/dev/" + macWifi + "/telemetry";

    // 1ª tentativa (se NTP ainda não sincronizou, pode falhar; reconecta depois)
        String cid = "premax-" + macWifi;
        mqttConnectWithLWT(cid);

    }
    else{
        Serial.println("[Wifi]     - Falha ao se conectar");
        lg.salvarLog("WIFI       - Falha ao se conectar.");
        digitalWrite(LED_NET, 0);
        logOn = 0;
    }
}
/* funçao que envia o json para o servidor via HTTPS comentada 
void Wifi::enviaJson(unsigned long id){
    if(WiFi.status() == WL_CONNECTED){
        /* (ALTERADO) HTTPS correto: usar WiFiClientSecure + http.begin(client, url) 
        WiFiClientSecure client;
        client.setInsecure(); // trocar por setCACert(...) em produção

        HTTPClient http;
        http.begin(client, server);

        http.addHeader("Content-Type", "application/json");

        // Criação do Json  
        JsonDocument jsonDoc;//StaticJsonDocument<200> jsonDoc;
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
    
        String requestBody;
        serializeJson(jsonDoc, requestBody);
    
        // Envia via PATCH (mantido) 
        int httpResponseCode = http.PATCH(requestBody);
    
        if(httpResponseCode == 200){
            Serial.println("[Wifi]     - Json enviado com sucesso! ");
            // Pisca LED de envio 
            for(int i = 0; i < 3; i++){
                digitalWrite(LED_SEND, 1);
                delay(50);
                digitalWrite(LED_SEND, 0);
                delay(400);
            }
            // zera contador de falhas de rede se desejar (opcional)
        } 
        else{
            Serial.println("[Wifi]     - Erro ao enviar!");
            Serial.print("[Wifi]     - Código de resposta: ");
            Serial.println(httpResponseCode);
            lg.salvarLog("WIFI       - Erro ao enviar json.");
        }
        http.end();
    } 
} */

/* Publica JSON via MQTT TLS */
void Wifi::enviaJson(unsigned long id){
  if(WiFi.status() != WL_CONNECTED) return;

  if(!mqtt.connected()){
    static unsigned long lastRetry = 0;
    if(millis() - lastRetry < 3000) return;
    lastRetry = millis();
    String cid = "premax-" + macWifi;
    mqttConnectWithLWT(cid);
    if(!mqtt.connected()) return;
  }

  JsonDocument jsonDoc; // se usar ArduinoJson v6, troque para StaticJsonDocument<1024>
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

  String payload; serializeJson(jsonDoc, payload);

  if(mqtt.publish(MQTT_TOPIC.c_str(), payload.c_str(), false)){
    Serial.println("[Wifi]     - MQTT TLS publish OK");
    for(int i=0;i<3;i++){ digitalWrite(LED_SEND,1); delay(50); digitalWrite(LED_SEND,0); delay(400); }
  } else {
    Serial.println("[Wifi]     - MQTT TLS publish FAIL");
    lg.salvarLog("WIFI       - Falha ao publicar MQTT.");
  }
}



void Wifi::wifiLoop(){
    if(WiFi.status() == WL_CONNECTED){
        if(!logOn){
            digitalWrite(LED_NET, 1);
            Serial.println("[Wifi]     - Wifi reconectado!");
            //lg.salvarLog("WIFI       - Wifi reconectado.");
            logOn = 1;
        }
        /* (NOVO) zera watchdog ao reconectar */
        wifiRetryCount = 0;
    }
    else{
        if(logOn){
            digitalWrite(LED_NET, 0);
            Serial.println("[Wifi]     - Wifi Desconectado");
            Serial.println("[Wifi]     - Tentando reconectar...");
            lg.salvarLog("WIFI       - Wifi desconectado.");
            logOn = 0;
        }

        /* (NOVO) tentativa ativa de reconexão + watchdog */
        WiFi.reconnect();
        wifiRetryCount++;

        /* (NOVO) após N tentativas, reabre o portal AP por tempo limitado */
        if(wifiRetryCount >= WIFI_MAX_RETRIES_BEFORE_AP){
            wifiRetryCount = 0;
            Serial.println("[Wifi]     - Abrindo portal de configuração (AP)...");
            bool ok = wm.startConfigPortal(SSID); // bloqueia até conectar ou timeout
            Serial.println(ok ? "[Wifi]     - Configurado via portal." :
                                 "[Wifi]     - Portal terminou sem conectar.");
        }

        wm.setWiFiAutoReconnect(true);
    }
}

void Wifi::mqttLoop(){
  if(WiFi.status()==WL_CONNECTED && mqtt.connected()){
    mqtt.loop();
  }
}
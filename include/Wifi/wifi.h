#ifndef _WIFI_H_
#define _WIFI_H_

#include <WiFi.h>
// #include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <PubSubClient.h>      // <-- MQTT
#include "Hardware/pin_map.h"

#define             SSID            "preMax"
#define             PASSWORD        "12345678"

class Wifi {
  public:
    String macWifi;
    Wifi();
    void begin();
    void enviaJson(unsigned long);
    void wifiLoop();
    void mqttLoop();               // <-- precisa ser chamado em toda iteração
};

#endif

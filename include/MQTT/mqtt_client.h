#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "MQTT/mqtt_config.h"

class MqttClient {
public:
  static MqttClient& instance();
  void begin(const String& mac);
  void loop();
  bool publish(const String& topic, const String& payload, int qos=0, bool retain=false);

private:
  MqttClient();
  bool ensureConnected();

  String clientId;
  String mac;

#if MQTT_USE_TLS
  WiFiClientSecure net;
#else
  WiFiClient net;
#endif
  PubSubClient ps;          // NOME ALTERADO: não use "cli" (conflita com macro Arduino)
  unsigned long lastRetry = 0;
};

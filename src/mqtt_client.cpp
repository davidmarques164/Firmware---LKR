#include "MQTT/mqtt_client.h"

MqttClient& MqttClient::instance(){ static MqttClient inst; return inst; }

MqttClient::MqttClient() : ps(net) {
  ps.setServer(MQTT_HOST, MQTT_PORT);
  ps.setKeepAlive(30);
  ps.setBufferSize(2048); // mais folga p/ JSON/log
}

void MqttClient::begin(const String& mac_) {
  mac = mac_;
#if MQTT_USE_TLS
  net.setCACert(MQTT_ROOT_CA_PEM);
#endif
  clientId = "premax-" + mac;
  ensureConnected();
}

bool MqttClient::ensureConnected() {
  if (ps.connected()) return true;
  if (millis() - lastRetry < 3000) return false;
  lastRetry = millis();

  String willTopic = "premax/status/" + clientId;
  const char* willMsg = "offline";
  const uint8_t willQos = 1; const bool willRet = true;

  if (strlen(MQTT_USER)) {
    return ps.connect(clientId.c_str(), MQTT_USER, MQTT_PASS,
                      willTopic.c_str(), willQos, willRet, willMsg);
  } else {
    return ps.connect(clientId.c_str(),
                      willTopic.c_str(), willQos, willRet, willMsg);
  }
}

void MqttClient::loop() {
  if (ps.connected()) ps.loop(); else ensureConnected();
}

bool MqttClient::publish(const String& topic, const String& payload, int qos, bool retain) {
  if (!ensureConnected()) return false;
  return ps.publish(topic.c_str(),
                    (const uint8_t*)payload.c_str(),
                    payload.length(), retain);
}

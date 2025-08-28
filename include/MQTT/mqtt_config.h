#pragma once

// 0 = TCP sem TLS (seu caso atual); 1 = TLS (MQTTS) com CA
#define MQTT_USE_TLS 0

#define MQTT_HOST "45.160.103.154"
#define MQTT_PORT 1885
#define MQTT_USER "premax"
#define MQTT_PASS "aut0r3aliz3"

// Só defina a CA se MQTT_USE_TLS == 1 (evita erro de raw string)
#if MQTT_USE_TLS
#define MQTT_ROOT_CA_PEM \
"-----BEGIN CERTIFICATE-----\n" \
"...COLE_AQUI_A_CA_DO_SEU_BROKER...\n" \
"-----END CERTIFICATE-----\n"
#endif

// Tópicos (se quiser dinâmicos por MAC)
#define TOPIC_TELEMETRY(mac) (String("premax/dev/") + (mac) + "/telemetry")
#define TOPIC_LOGS(mac)      (String("premax/dev/") + (mac) + "/logs")

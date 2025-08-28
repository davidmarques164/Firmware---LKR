#include <Log/log.h>
#include <Wifi/wifi.h>
#include "NTP/ntp.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "MQTT/mqtt_config.h"  // CONFIG ÚNICA de host/porta/credenciais/TLS

extern ReadTime ntp;
extern Wifi     wf;      // usa wf.macWifi para compor o tópico

#define LOG_FILE "/log.txt"

Log::Log(){}

void Log::begin(){
    if(!SPIFFS.begin(true)){
        Serial.println("[Log]      - Falha ao montar SPIFFS");
        return;
    }
    Serial.println("[Log]      - SPIFFS montado com sucesso!");
    apagarLogs();                  // mesmo comportamento do código atual
    //salvarLog("INIT       - Sistema inicializado.");
    mostrarLogs();
}

void Log::limitaLinhas(int linhasLimite){
    File arquivo = SPIFFS.open(LOG_FILE, FILE_READ);
    if(!arquivo){
        Serial.println("[Log]      - Falha ao abrir o arquivo para leitura!");
        return;
    }
    std::vector<String> linhas;
    while(arquivo.available()){
        String l = arquivo.readStringUntil('\n');
        l.trim();
        if(l.length() > 0) linhas.push_back(l);
    }
    arquivo.close();

    while((int)linhas.size() > linhasLimite) linhas.erase(linhas.begin());

    arquivo = SPIFFS.open(LOG_FILE, FILE_WRITE);
    if(!arquivo){
        Serial.println("[Log]      - Falha ao abrir o arquivo para escrita!");
        return;
    }
    for(const auto& linha : linhas) arquivo.println(linha);
    arquivo.close();
}

void Log::salvarLog(String mensagem){
    mensagem.replace("\n", "");
    mensagem.replace("\r", "");

    File arquivo = SPIFFS.open(LOG_FILE, FILE_APPEND);
    if(!arquivo){
        Serial.println("[Log]      - Falha ao abrir o arquivo de log!");
        return;
    }
    String linha = "[" + ntp.formattedDateTime() + "] " + mensagem;
    arquivo.println(linha);
    arquivo.close();
    Serial.println("[Log]      - Log salvo: " + linha);
}

void Log::mostrarLogs(){
    File arquivo = SPIFFS.open(LOG_FILE, FILE_READ);
    if(!arquivo){
        Serial.println("[Log]      - Não foi possível abrir o log!");
        return;
    }
    Serial.println("[Log]      - Conteúdo do log:");
    while(arquivo.available()){
        String linha = arquivo.readStringUntil('\n');
        Serial.println("             " + linha);
    }
    arquivo.close();
}

void Log::apagarLogs(){
    SPIFFS.remove(LOG_FILE);
    Serial.println("[Log]      - Logs apagados!");
}

void Log::enviarLog(){
    if(WiFi.status() != WL_CONNECTED){
        Serial.println("[Log]      - Sem Wi-Fi; não enviando.");
        return;
    }

    limitaLinhas(10);
    if(!SPIFFS.exists(LOG_FILE)){
        Serial.println("[Log]      - O arquivo log.txt não existe na memória!");
        return;
    }

    File logFile = SPIFFS.open(LOG_FILE, FILE_READ);
    if(!logFile){
        Serial.println("[Log]      - Falha ao abrir log.txt");
        return;
    }
    String conteudo = logFile.readString();
    logFile.close();

    // Tópico por dispositivo
    String topic = String("premax/dev/") + wf.macWifi + "/logs";

    // Cliente de rede conforme configuração
#if MQTT_USE_TLS
    WiFiClientSecure net;
    net.setCACert(MQTT_ROOT_CA_PEM);      // valida broker (produção)
#else
    WiFiClient net;                       // TCP sem TLS
#endif

    PubSubClient mqtt(net);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setBufferSize(2048);             // log pode ser >256B
    mqtt.setKeepAlive(15);

    // Conexão com LWT (mesmo padrão do Wi-Fi)
    String cid = String("premax-log-") + wf.macWifi;
    String willTopic = String("premax/status/") + cid;
    const char* willMsg = "offline";
    bool ok;
    if (strlen(MQTT_USER) > 0) {
        ok = mqtt.connect(cid.c_str(), MQTT_USER, MQTT_PASS,
                          willTopic.c_str(), 1, true, willMsg);
    } else {
        ok = mqtt.connect(cid.c_str(), willTopic.c_str(), 1, true, willMsg);
    }
    if(!ok){
        Serial.println("[Log]      - MQTT: falha ao conectar broker");
        return;
    }

    if(mqtt.publish(topic.c_str(), (const uint8_t*)conteudo.c_str(), conteudo.length(), false)){
        Serial.println("[Log]      - Log enviado via MQTT");
    } else {
        Serial.println("[Log]      - Falha ao publicar log via MQTT (buffer/conexão?)");
    }
    mqtt.disconnect();
}

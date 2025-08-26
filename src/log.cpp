#include <Log/log.h>
#include <Wifi/wifi.h>
#include "NTP/ntp.h"

#include <WiFi.h>               // <-- adicionado para MQTT
#include <WiFiClientSecure.h>   // <-- adicionado para MQTT (TLS)
#include <PubSubClient.h>       // <-- adicionado para MQTT

extern      ReadTime    ntp;
extern      Wifi        wf;     // usar MAC/tópico

#define     LOG_FILE    "/log.txt"

// const char* firebaseUploadURL = "https://firebasestorage.googleapis.com/v0/b/eduardo-d28ce.appspot.com/o/preMax%2Flogs.txt?uploadType=media&name=logs/log.txt";
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// (HTTP desativado; envio agora será via MQTT TLS)

// ===== MQTT (TLS) para LOG =====
static const char* LOG_MQTT_HOST = "broker.emqx.io"; // broker público p/ testes
static const uint16_t LOG_MQTT_PORT = 8883;          // TLS
// Se quiser autenticação no broker, defina aqui:
// static const char* LOG_MQTT_USER = "usuario";
// static const char* LOG_MQTT_PASS = "senha";

// (Para produção, substitua por CA real e use setCACert; neste exemplo usaremos setInsecure para teste.)

Log::Log(){}
void Log::begin(){
    /* Monta SPIFFS */
    if(!SPIFFS.begin(true)){
        Serial.println("[Log]      - Falha ao montar SPIFFS");
        return;
    }
    Serial.println("[Log]      - SPIFFS montado com sucesso!");
    apagarLogs();
    //salvarLog("INIT       - Sistema inicializado.");
    mostrarLogs();
}
void Log::limitaLinhas(int linhasLimite){
    File arquivo = SPIFFS.open(LOG_FILE, FILE_READ);
    if(!arquivo){
        Serial.println("[Log]      - Falha ao abrir o arquivo para leitura!");
        return;
    }

    /* Armazena todas as linhas */ 
    std::vector<String> linhas;
    while(arquivo.available()){
        String l = arquivo.readStringUntil('\n');
        l.trim();  //Remove espaços e quebras extras
        if(l.length() > 0) linhas.push_back(l);
    }
    arquivo.close();

    /* Se tiver muitas linhas, remove do começo */ 
    while(linhas.size() > linhasLimite) linhas.erase(linhas.begin());

    /* Reescreve o arquivo */ 
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
    limitaLinhas(10);
    if(!SPIFFS.exists("/log.txt")){
      Serial.println("[Log]      - O arquivo log.txt não existe na memória!");
      return;
    }
    File logFile = SPIFFS.open("/log.txt", FILE_READ);
    if(!logFile){
      Serial.println("[Log]      - Falha ao abrir log.txt");
      return;
    }

    // =========================
    // ENVIO HTTP (DESATIVADO)
    // =========================
    /*
    HTTPClient http;
    http.begin(firebaseUploadURL);
    http.addHeader("Content-Type", "text/plain");
    int tamanho = logFile.size();
    String conteudo = logFile.readString();  
    int httpResponseCode = http.POST(conteudo);
    if(httpResponseCode > 0){
      Serial.println("[Log]      - Enviado!");
    } 
    else{
      Serial.printf("[Log]      - Erro no envio: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
    logFile.close();
    return;
    */

    // =========================
    // ENVIO MQTT (ATIVO)
    // =========================

    String conteudo = logFile.readString();
    logFile.close();

    // Tópico usa o MAC do dispositivo
    String topic = "premax/dev/" + wf.macWifi + "/logs";

    // Cliente TLS ad-hoc para o envio do log
    WiFiClientSecure tls;
    // Em produção, use setCACert(CA_PEM) para validar o broker.
    tls.setInsecure(); // TESTE APENAS

    PubSubClient mqtt(tls);
    mqtt.setServer(LOG_MQTT_HOST, LOG_MQTT_PORT);
    mqtt.setBufferSize(2048);   // log pode ser maior que 256B
    mqtt.setKeepAlive(15);      // suficiente para publish único

    // Conecta no broker (sem auth)
    String cid = "premax-log-" + wf.macWifi;
    bool ok = mqtt.connect(cid.c_str());
    if(!ok){
      Serial.println("[Log]      - MQTT TLS: falha ao conectar broker");
      return;
    }

    // Publica o log
    if(mqtt.publish(topic.c_str(), conteudo.c_str(), false)){
      Serial.println("[Log]      - Log enviado via MQTT TLS");
    } else {
      Serial.println("[Log]      - Falha ao publicar log via MQTT TLS (buffer/conexão?)");
    }

    mqtt.disconnect();
}

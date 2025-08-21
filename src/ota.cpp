#include "OTA/ota.h"
#include "Log/log.h"

extern        Log             lg;

updateOTA::updateOTA(){}
void updateOTA::updateFirmware(){ 
    HTTPClient http;
    http.begin(FIRMWARE_URL);
    int httpCode = http.GET();
    if(httpCode == HTTP_CODE_OK){
        int contentLength = http.getSize();
        WiFiClient *stream = http.getStreamPtr();
        if(contentLength > 0){
            Serial.print("[OTA]      - Baixando novo firmware...");
            if(Update.begin(contentLength)){
                size_t written = Update.writeStream(*stream);
                if(written == contentLength){
                    Serial.println("  Download concluído!");
                    if(Update.end()){
                        Serial.println("[OTA]      - Atualização bem-sucedida! Reiniciando...");
                        //lg.salvarLog("OTA        - Atualizacao de firmware bem-sucedida.");
                        ESP.restart();
                    } 
                    else{
                        Serial.println("[OTA]      - Erro ao finalizar atualização.");
                        lg.salvarLog("OTA        - Erro ao finalizar atualizacao de firmware.");
                    }
                } 
                else{
                    Serial.println("  Erro: Tamanho do arquivo incorreto.");
                    lg.salvarLog("OTA        - Erro de tamanho incorreto de arquivo.");
                }
            } 
            else{
                Serial.println("  Falha ao iniciar atualização.");
                lg.salvarLog("OTA        - Falha ao iniciar atualizacao de firmware.");
            }
        }
    } 
    else{
        Serial.println("[OTA]      - Erro ao baixar firmware.");
        lg.salvarLog("OTA        - Erro ao baixar firmware.");
    }
    http.end();
}
void updateOTA::checkForUpdate(){ 
    Serial.print("[OTA]      - Verificando nova versão de firmware... ");
    HTTPClient http;
    http.begin(VERSION_URL);
    int httpCode = http.GET();
    if(httpCode == HTTP_CODE_OK){
        String newVersion = http.getString();
        newVersion.trim();  // Remover espaços extras
        Serial.println(newVersion);
        if(newVersion != VERSION){
            Serial.println("[OTA]      - Nova versão disponível! Iniciando atualização...");
            updateFirmware();
        } 
        else Serial.println("[OTA]      - Já está na versão mais recente.");
    } 
    else{
        Serial.println("[OTA]      - Falha ao obter a versão do servidor.");
        lg.salvarLog("OTA        - Falha ao obter a versao do firmware do servidor.");
    }
    http.end();
}
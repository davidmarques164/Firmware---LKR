#include <Arduino.h>
#include "Hardware/pin_map.h"
#include "NTP/ntp.h"
#include "Wifi/wifi.h"
#include "ConfigPins/config_pins.h"
#include "ModbusTcp/modbusTcp.h"
#include "OTA/ota.h"
#include "Log/log.h"

Wifi            wf;
ReadTime        ntp;
ConfigPins      modePins;
ModbusClp       clp;
// ModbusIhm ihm;   // <== Preparado, para uso futuro
updateOTA       ota;
Log             lg;

#define         TIME_READ_CLP_S                 10 
#define         TIME_SEND_JSON_S                20
#define         TIME_UPDATE_FIRMWARE_S          5000
#define         TIME_VERIFY_WIFI_S              10
#define         TIME_SEND_LOG_S                 10
#define         TIME_ERASE_LOG_MIN              30
// #define         TIME_READ_IHM_S  5   // <== reservado p/ IHM

unsigned long id = 0,
              lastTime1 = 0, 
              lastTime2 = 0, 
              lastTime3 = 0, 
              lastTime4 = 0, 
              lastTime5 = 0, 
              lastTime6 = 0, 
              lastTime7 = 0 /*,
              lastTime8 = 0*/; // Variáveis de tempo para controle de leitura do IHM se um dia necessário.

TaskHandle_t core0Handle;

void core0(void *parameter){
  while (true) {
    /*if(millis() - lastTime7 >= 200){
      lastTime7 = millis();
      digitalWrite(LED_SEND, !digitalRead(LED_SEND));
     }*/
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void setup(){
  /* Inicializa serial */
  Serial.begin(115200);
  /* Inicializa wifi */
  wf.begin();
  /* Inicializa modbus */
  clp.begin(); // Inicializa comunicação Modbus clp
  /* Inicializa NTP */
  // ihm.begin();// Inicializa comunicação Modbus ihm
  /* Inicializa NTP */
  ntp.begin();
  /* Configura o modo de cada pino */
  modePins.begin();
  /* Inicializa arquivo de log */
  lg.begin();
  /* Cria tarefa no core 0 */
  xTaskCreatePinnedToCore(core0, "Core0", 1024, NULL, 1, &core0Handle, 0);
}

void loop(){
   /* Ler CLP */
   if(millis() - lastTime1 >= TIME_READ_CLP_S * 1000){
    lastTime1 = millis();
    clp.lerClp(); // Lê dados do CLP via Modbus
   }
  /* Envia Json */
  if(millis() - lastTime2 >= TIME_SEND_JSON_S * 1000){
    lastTime2 = millis();
    id++;
    wf.enviaJson(id);
  }
  /* Verifica atualização de Firmware */
  if(millis() - lastTime3 >= TIME_UPDATE_FIRMWARE_S * 1000){
    lastTime3 = millis();
    ota.checkForUpdate();
  }
  /* Verifica conexão wifi */
  if(millis() - lastTime4 >= TIME_VERIFY_WIFI_S * 1000){
    lastTime4 = millis();
    wf.wifiLoop();
  }
  /* Envia log */
  if(millis() - lastTime5 >= TIME_SEND_LOG_S * 1000){
    lastTime5 = millis();
    lg.enviarLog();
  }
  /* Apaga logs */
  /*if(millis() - lastTime6 >= TIME_ERASE_LOG_MIN * 60000){
    lastTime6 = millis();
    lg.apagarLogs();
  }*/
   /*
  if (millis() - lastTime8 >= TIME_READ_IHM_S * 1000) {
    lastTime8 = millis();
    ihm.lerIhm();
  }
  */

  wf.mqttLoop();  // mantém a sessão TLS viva

}

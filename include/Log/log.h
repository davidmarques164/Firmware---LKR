#ifndef _LOG_H_
#define _LOG_H_

#include <Arduino.h>
#include <SPIFFS.h>

class Log {
  public:
    Log();
    void begin();
    void limitaLinhas(int);
    void salvarLog(String);
    void mostrarLogs();
    void apagarLogs();
    void enviarLog();
};

#endif

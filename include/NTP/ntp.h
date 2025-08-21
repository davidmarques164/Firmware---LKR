#ifndef _NTP_H_
#define _NTP_H_

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class ReadTime {
  public:
    ReadTime();
    void begin();
    String formattedDateTime();
};

#endif

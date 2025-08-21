#include "NTP/ntp.h"

WiFiUDP             ntpUDP;
NTPClient           timeClient(ntpUDP, "pool.ntp.org", -10800);  // UTC-3 (Brasil)

ReadTime::ReadTime(){}
void ReadTime::begin(){
    timeClient.begin(); //Inicia cliente NTP/UDP
    timeClient.forceUpdate(); //Atualiza data e hora
}
String ReadTime::formattedDateTime(){ 
    /* Formato desejado: 22/03/2025 12:40:00, como chega: 2023-03-26T02:52:43Z */
    String valorAtual = timeClient.getFormattedDate();
    String strFormatted = valorAtual.substring(valorAtual.indexOf("T") - 2, valorAtual.indexOf("T")) + "/" + valorAtual.substring(valorAtual.indexOf("T") - 5, valorAtual.indexOf("T") - 3) + "/" + valorAtual.substring(0, 4) + " " + valorAtual.substring(valorAtual.indexOf("T") + 1, valorAtual.indexOf("Z"));
    return strFormatted;
  }
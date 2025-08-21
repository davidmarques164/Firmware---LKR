#include "ConfigPins/config_pins.h"

ConfigPins::ConfigPins(){}
void ConfigPins::begin(){
    /* Entradas */
    pinMode(S_01, INPUT);
    pinMode(S_02, INPUT);
    pinMode(S_03, INPUT);
    pinMode(S_04, INPUT);
    pinMode(S_05, INPUT);
    pinMode(S_06, INPUT);
    pinMode(S_07, INPUT);
    /* Saídas */
    pinMode(LED_NET, OUTPUT);
    pinMode(LED_SEND, OUTPUT);
}
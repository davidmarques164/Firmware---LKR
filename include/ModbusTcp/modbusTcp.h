#ifndef _MODBUS_TCP_H_
#define _MODBUS_TCP_H_

#include <Ethernet.h>
#include <ModbusEthernet.h>
#include "Hardware/pin_map.h"
#include <Arduino.h>

class ModbusClp {
  private:
    // Endereços do PLC (Holding Registers 400100..400106)
    const uint16_t address_pallets_produzidos,
                   address_pallets_produzidos_total,
                   address_tabuas_empilhadas_1,
                   address_tabuas_empilhadas_2,
                   address_tempo_parada_ciclo_entrada,
                   address_tempo_parada_ciclo_cuber,
                   address_tempo_parada_ciclo_saida;

    byte mac[6];
    unsigned long lastTime1, lastTime2, lastTime3;

    // IPs envolvidos
    IPAddress ipClp;   // PLC
    IPAddress ipEsp;   // ESP/TLE
    // IPAddress ipIhm;   // IHM (não usado ainda)

  public:
    // Variáveis recebidas do PLC
    uint16_t pallets_produzidos,
             pallets_produzidos_total,
             tabuas_empilhadas_1,
             tabuas_empilhadas_2,
             tempo_parada_ciclo_entrada,
             tempo_parada_ciclo_cuber,
             tempo_parada_ciclo_saida;

    ModbusClp();

    void wizReset();
    void begin();
    void lerClp();

    // Futuro: leitura da IHM
    // void lerIhm();  // declarado mas ainda não implementado
};

/* class ModbusIhm {
  private:
    byte       mac[6];
    IPAddress  ipEsp;     // IP do ESP/TLE
    IPAddress  ipIhm;     // IP da IHM

  public:
    ModbusIhm();

    void begin();   // (Preparado) Inicializa/garante conexão Modbus c/ IHM
    void lerIhm();  // (Preparado) Leitura periódica de registradores da IHM
};
*/ 

#endif

#include "ModbusTcp/modbusTcp.h"
#include "Log/log.h"

ModbusEthernet mb;
extern Log lg;

bool logOn1 = 1, logOn2 = 1, logOn3 = 1, logOn4 = 1;

/*
 * IPs:
 *  - ESP/TLE: 192.168.15.10
 *  - PLC:     192.168.15.11
 *  - IHM:     192.168.15.12 (reservado, ainda não usado)
 * 
 * Holding Registers do PLC:
 *  - 400100..400106 -> índices 99..105 (zero-based)
 */
ModbusClp::ModbusClp()
  : //ipClp(192, 168, 15, 11), //CLP ORIGINAL
    ipClp(192, 168, 0, 1), //CLP teste
    ipEsp(192, 168, 0, 10), 
    // ipIhm(192, 168, 15, 12),
    
    //ENDEREÇOS REAIS

    /*pallets_produzidos(0),
    pallets_produzidos_total(0),
    tabuas_empilhadas_1(0),
    tabuas_empilhadas_2(0),
    tempo_parada_ciclo_entrada(0),
    tempo_parada_ciclo_cuber(0),
    tempo_parada_ciclo_saida(0),
    address_pallets_produzidos(99),
    address_pallets_produzidos_total(100),
    address_tabuas_empilhadas_1(101),
    address_tabuas_empilhadas_2(102),
    address_tempo_parada_ciclo_entrada(103),
    address_tempo_parada_ciclo_cuber(104),
    address_tempo_parada_ciclo_saida(105)*/

    // ENDEREÇO TESTE
    address_pallets_produzidos(0),
    address_pallets_produzidos_total(1),
    address_tabuas_empilhadas_1(2),
    address_tabuas_empilhadas_2(3),
    address_tempo_parada_ciclo_entrada(4),
    address_tempo_parada_ciclo_cuber(20),
    address_tempo_parada_ciclo_saida(21),
    pallets_produzidos(22),
    pallets_produzidos_total(23),
    tabuas_empilhadas_1(24)

{
    byte default_mac[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    memcpy(mac, default_mac, 6);
}

void ModbusClp::wizReset() {
    Serial.print("[Ethernet] - Resetando placa Wiz W5500 Ethernet...");
    pinMode(RESET, OUTPUT);
    digitalWrite(RESET, HIGH);
    delay(250);
    digitalWrite(RESET, LOW);
    delay(50);
    digitalWrite(RESET, HIGH);
    delay(350);
    Serial.println(" Feito!");
}

void ModbusClp::begin() {
    mb.client();
    Ethernet.init(CS);
    wizReset();
    Ethernet.begin(mac, ipEsp);  // IP fixo do ESP/TLE
    Serial.println("[Ethernet] - Ethernet com IP fixo configurado.");
    Serial.print("[Ethernet] - IP local: ");
    Serial.println(Ethernet.localIP());

    // Conecta inicialmente ao PLC
    mb.connect(ipClp);

    // Futuro: conexão com IHM pode ser habilitada aqui:
    // mb.connect(ipIhm);
}

void ModbusClp::lerClp() {
    if(Ethernet.hardwareStatus() != EthernetNoHardware){
      logOn3 = 1;
      if(Ethernet.linkStatus() != LinkOFF){
        logOn2 = 1;
        Serial.print("[CLP]      - Lendo CLP...");
        if(mb.isConnected(ipClp)){
          logOn1 = 1;
          Serial.println("");
          mb.readHreg(ipClp, address_pallets_produzidos, &pallets_produzidos);
          mb.readHreg(ipClp, address_pallets_produzidos_total, &pallets_produzidos_total);
          mb.readHreg(ipClp, address_tabuas_empilhadas_1, &tabuas_empilhadas_1);
          mb.readHreg(ipClp, address_tabuas_empilhadas_2, &tabuas_empilhadas_2);
          mb.readHreg(ipClp, address_tempo_parada_ciclo_entrada, &tempo_parada_ciclo_entrada);
          mb.readHreg(ipClp, address_tempo_parada_ciclo_cuber, &tempo_parada_ciclo_cuber);
          mb.readHreg(ipClp, address_tempo_parada_ciclo_saida, &tempo_parada_ciclo_saida);

          Serial.println("             * pallets_produzidos: " + String(pallets_produzidos));
          Serial.println("             * pallets_produzidos_total: " + String(pallets_produzidos_total));
          Serial.println("             * tabuas_empilhadas_1: " + String(tabuas_empilhadas_1));
          Serial.println("             * tabuas_empilhadas_2: " + String(tabuas_empilhadas_2));
          Serial.println("             * tempo_parada_ciclo_entrada: " + String(tempo_parada_ciclo_entrada));
          Serial.println("             * tempo_parada_ciclo_cuber: " + String(tempo_parada_ciclo_cuber));
          Serial.println("             * tempo_parada_ciclo_saida: " + String(tempo_parada_ciclo_saida));
        }
        else{
          mb.connect(ipClp);
          if(logOn1){
            Serial.print(" Erro ao ler CLP! Reconectando...");
            lg.salvarLog("ETHERNET   - Erro ao ler CLP.");
            logOn1 = 0;
          }
          logOn4 = 1;
          Serial.println("");
        }
      }
      else{
        if(logOn2){
          Serial.println("[Ethernet] - Cabo Ethernet não conectado.");
          lg.salvarLog("ETHERNET   - Cabo Ethernet nao conectado.");
          logOn2 = 0;
        }
        Ethernet.begin(mac, ipEsp);
      }
    }
    else{
      if(logOn3){
        Serial.println("[Ethernet] - Hardware Ethernet não encontrado.");
        lg.salvarLog("ETHERNET   - Hardware Ethernet nao encontrado.");
        logOn3 = 0;
      }
      Ethernet.begin(mac, ipEsp);
    }
}

/*
// Placeholder: futuro uso da IHM
void ModbusClp::lerIhm() {
    // Exemplo de leitura futura:
    // if(mb.isConnected(ipIhm)) {
    //     uint16_t var_temp;
    //     mb.readHreg(ipIhm, 0, &var_temp);
    // }
}

ModbusIhm::ModbusIhm()
  : ipEsp(192, 168, 15, 10),
    ipIhm(192, 168, 15, 12)
{
    byte default_mac[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
    memcpy(mac, default_mac, 6);
}

void ModbusIhm::begin(){
    // Garante pilha e Ethernet ativas (idempotente se já inicializadas pelo CLP)
    mb.client();
    Ethernet.init(CS);

    if (Ethernet.hardwareStatus() == EthernetNoHardware){
        Serial.println("[IHM]      - Ethernet HW nao encontrado. Inicializando...");
    }

    // Evita reset duplo do W5500: assume que ModbusClp::begin já resetou.
    // Se usar somente IHM sem CLP, descomente um reset aqui conforme necessário.

    // Garante IP local do ESP (caso este objeto seja usado isoladamente)
    if (Ethernet.localIP() == IPAddress(0,0,0,0)){
        Ethernet.begin(mac, ipEsp);
        Serial.print  ("[IHM]      - IP local (ESP): ");
        Serial.println(Ethernet.localIP());
    }

    // Conecta à IHM (somente quando houver leituras definidas)
    mb.connect(ipIhm);
    Serial.print  ("[IHM]      - Conectando a IHM em ");
    Serial.println(ipIhm);
}

void ModbusIhm::lerIhm(){
    if (Ethernet.hardwareStatus() == EthernetNoHardware || Ethernet.linkStatus() == LinkOFF){
        // Tenta manter a interface viva
        Ethernet.begin(mac, ipEsp);
        return;
    }
    if (!mb.isConnected(ipIhm)){
        mb.connect(ipIhm);
        return;
    }

    // ===== EXEMPLO — preencha quando tiver mapeamento de registradores da IHM =====
    // uint16_t algum_reg;
    // mb.readHreg(ipIhm, <indice_zero_based>, &algum_reg);
    // Serial.println(String("[IHM]      - algum_reg: ") + algum_reg);
    // ===============================================================================
}
    */
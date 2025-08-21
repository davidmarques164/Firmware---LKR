# preMax – Firmware (Resumo Técnico)

## Arquitetura (fluxo)

1. **Wi-Fi** conecta via *WiFiManager*; acende `LED_NET` ao conectar.
2. **NTP** sincroniza hora (UTC-3) e fornece `dd/MM/yyyy HH:mm:ss`.
3. **ModbusTCP** (W5500) lê registradores do CLP em IP `192.168.0.10` (ESP `192.168.0.5`).
4. **Loop** periódico:

   * Lê CLP a cada **10s**.
   * Envia JSON a cada **20s** para Firebase (HTTP **PATCH** em `.../preMax.json`).
   * Verifica **OTA** a cada **5000s** (compara `version.txt`; baixa `firmware.bin` se diferente).
   * Verifica Wi-Fi a cada **10s**.
   * Envia **logs** a cada **10s** (SPIFFS → Firebase).

---

## Pinos (ESP32)

Entradas digitais: `S_01=13, S_02=12, S_03=14, S_04=27, S_05=26, S_06=25, S_07=33`.
Saídas: `LED_NET=2`, `LED_SEND=4`.
Ethernet W5500: `CS=5, CLK=18, MISO=19, MOSI=23, RESET=22`.
Inicialização de modo (entradas/saídas) realizada em `ConfigPins::begin()`.

---

## ModbusTCP (CLP)

* Reset W5500 via `wizReset()` e Ethernet com IP fixo.
* Endereços lidos (holding registers):

  * `0` → `pallets_produzidos`
  * `1` → `pallets_produzidos_total`
  * `2` → `tabuas_empilhadas_1`
  * `3` → `tabuas_empilhadas_2`
  * `4` → `tempo_parada_ciclo_entrada`
  * `5` → `tempo_parada_ciclo_cuber`
  * `6` → `tempo_parada_ciclo_saida`.

---

## NTP

Servidor base `pool.ntp.org`, *offset* `-10800` (UTC-3). `formattedDateTime()` reestrutura `YYYY-MM-DDTHH:MM:SSZ` para `dd/MM/yyyy HH:mm:ss`.

---

## Telemetria (HTTP → Firebase)

* Endpoint: `https://eduardo-d28ce.firebaseio.com/preMax.json` (método **PATCH**).
* Payload inclui: `id`, `mac`, `date_time`, variáveis do CLP e **entradas digitais S\_01..S\_07**.
* Feedback de envio: piscas em `LED_SEND`.

### Exemplo de JSON (da sua captura)

```json
{
  "date_time": "25/04/2025 13:44:21",
  "entrada_1": 0,
  "entrada_2": 0,
  "entrada_3": 0,
  "entrada_4": 0,
  "entrada_5": 1,
  "entrada_6": 0,
  "entrada_7": 0,
  "id": 16,
  "mac": "E4:65:B8:26:8B:40",
  "pallets_produzidos": 0,
  "pallets_produzidos_total": 0,
  "tabuas_empilhadas_1": 0,
  "tabuas_empilhadas_2": 0,
  "tempo_parada_ciclo_cubern": 0,
  "tempo_parada_ciclo_entrada": 0,
  "tempo_parada_ciclo_saida": 0
}
```

---

## OTA

* `VERSION = "1.0.0"` no firmware.
* Verifica `VERSION_URL` (texto); se diferente, baixa `FIRMWARE_URL` e aplica via `Update` → `ESP.restart()`.

---

## Logs (SPIFFS)

* Arquivo: `/log.txt`.
* Funções: limitar linhas, salvar, mostrar, apagar, enviar (HTTP POST para Firebase Storage).
* Envio automático a cada **10s** (com limitação de linhas).

---

## Tabelas rápidas

### Temporizações (segundos)

| Ação              | Intervalo |
| ----------------- | --------- |
| Leitura CLP       | 10s       |
| Envio JSON        | 20s       |
| Verificação OTA   | 5000s     |
| Verificação Wi-Fi | 10s       |
| Envio de Logs     | 10s       |

### Entradas incluídas no JSON

`entrada_1..entrada_7` ← `digitalRead(S_01..S_07)`.

---

## Arquivos chave

`main.cpp` (orquestra loop/timers); `wifi.cpp` (conexão/telemetria); `modbusTcp.cpp` (CLP via W5500); `ntp.cpp` (hora); `ota.cpp` (atualização); `log.cpp` (logs); `pin_map.h` (pinos); `configPins.cpp` (modes).

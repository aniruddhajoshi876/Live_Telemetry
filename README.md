# SX1276 LoRa Driver - STM32

A lightweight C++ driver for the Semtech SX1276 LoRa transceiver, written for STM32 microcontrollers using HAL and SPI.

---

## What is LoRa

LoRa (Long Range) is a spread spectrum modulation technique that intentionally spreads a signal over a wider bandwidth than the data actually needs. This gives it exceptional range and noise rejection at the cost of data rate.

Key terms:

- **Chip** - smallest unit of the transmitted waveform
- **Symbol** - a group of chips; one symbol carries log2(SF) bits
- **Spreading Factor (SF)** - how many chips represent one symbol. Higher SF = longer range, lower data rate
- **Bandwidth (BW)** - how wide in frequency the signal is. Wider BW = faster data rate, less range
- **Coding Rate (CR)** - ratio of data bits to total bits sent. 4/5 = least overhead, 4/8 = most error correction
- **Sensitivity** - the weakest signal the receiver can still successfully decode

---

## Hardware

### What You Need

- STM32 microcontroller (tested on STM32G491)
- SX1276 module (e.g. Ra-02 breakout)
- 915 MHz antenna

### Wiring

```
SPI LINES
---------
SX1276 SCK   →  STM32 SPI_SCK
SX1276 MOSI  →  STM32 SPI_MOSI
SX1276 MISO  →  STM32 SPI_MISO
SX1276 NSS   →  STM32 GPIO output (chip select)

POWER
-----
SX1276 3.3V  →  3.3V
SX1276 GND   →  GND

CONTROL
-------
SX1276 RESET →  STM32 GPIO output (pull low to reset)

ANTENNA
-------
SX1276 ANT   →  915 MHz antenna
```

> **WARNING:** SX1276 is 3.3V only. Do not connect to 5V pins.
> **WARNING:** Never transmit without an antenna - it will damage the PA.

DIO pins are not required for this driver since it uses SPI polling instead of interrupts. for now. i will add dma/interupts once i have tested radio works

---

## Architecture

### Signal Path

**RX:**
```
Antenna → LNA → Mixer/PLL → ADC (I/Q) → Decimation → Low Pass Filter → LoRa Demodulator → FIFO → SPI → MCU
```

**TX:**
```
MCU → SPI → FIFO → Packet Engine → LoRa Modulator → DAC → Mixer/PLL → Power Amplifier → Antenna
```

### LoRa Packet Format
```
[ Preamble ][ Sync Word ][ Header ][ Payload ][ CRC ]
```

- **Preamble** - synchronizes the receiver with the incoming signal
- **Sync Word** - marks the start of data; `0x12` = private network, `0x34` = LoRaWAN
- **Header** - contains payload length, coding rate, and CRC flag (explicit header mode)
- **Payload** - your actual data
- **CRC** - optional error check on the payload

---

## SPI Interface

The SX1276 communicates over SPI Mode 0 (CPOL=0, CPHA=0), MSB first, up to 10 MHz.

Three access modes exist:

- **Single** - one address byte + one data byte per transaction
- **Burst** - one address byte followed by multiple data bytes; address auto-increments
- **FIFO** - address points to FIFO; address does not increment between bytes

### Register Access Format

**Write:**
```
MOSI: [ address | 0x80 ] [ value ]
MISO: [ ignored        ] [ ignored ]
```

**Read:**
```
MOSI: [ address & 0x7F ] [ dummy byte ]
MISO: [ ignored        ] [ register value ]
```

Bit 7 of the address byte controls direction: `1` = write, `0` = read.

---

## FIFO

The SX1276 has a 256-byte shared FIFO for TX and RX data.

- Not accessible in Sleep mode
- Data persists until overwritten
- Accessed via `RegFifoAddrPtr` (0x0D) which must be set before every read/write
- The pointer auto-increments on each byte access

Key FIFO registers:

| Register | Description |
|----------|-------------|
| `REG_FIFO` (0x00) | SPI gateway into the FIFO buffer |
| `REG_FIFO_ADDR_PTR` (0x0D) | Current read/write pointer |
| `REG_FIFO_TX_BASE_ADDR` (0x0E) | Start of TX region (default 0x80) |
| `REG_FIFO_RX_BASE_ADDR` (0x0F) | Start of RX region (default 0x00) |
| `REG_FIFO_RX_CURRENT_ADDR` (0x10) | Start address of last received packet |
| `REG_RX_NB_BYTES` (0x13) | Number of bytes in last received packet |
| `REG_PAYLOAD_LENGTH_LORA` (0x22) | Number of bytes to transmit |

---

## IRQ Flags

`RegIrqFlags` (0x12) - write 1 to a bit to clear it:

```
bit 7 → RxTimeout
bit 6 → RxDone
bit 5 → PayloadCrcError
bit 4 → ValidHeader
bit 3 → TxDone
bit 2 → CadDone
bit 1 → FhssChangeChannel
bit 0 → CadDetected
```

### CRC Handling

- `RegModemConfig2.RxPayloadCrcOn` controls whether CRC is appended on TX
- On RX the receiver checks `RegHopChannel.CrcOnPayload` to know if a CRC is present
- If CRC is present and fails, `RegIrqFlags.PayloadCrcError` is set

---

## Operating Modes

`RegOpMode` (0x01):

```
bit 7 → LongRangeMode (0=FSK/OOK, 1=LoRa) - change only in Sleep
bit 6 → ModulationType MSB
bit 5 → ModulationType LSB (00=FSK, 01=OOK)
bit 4 → Reserved
bit 3 → LowFrequencyModeOn
bit 2 → Mode bit 2
bit 1 → Mode bit 1
bit 0 → Mode bit 0
```

Mode values:

```
000 = Sleep
001 = Standby
010 = FSTx  (PLL on, RF off, TX freq)
011 = TX    (transmitting)
100 = FSRx  (PLL on, RF off, RX freq)
101 = RxContinuous
110 = RxSingle
111 = CAD
```

---

## Startup Sequence

```
1. Reset chip (pull RESET low)
2. Enter Sleep mode
3. Verify chip version over SPI (RegVersion should read 0x12)
4. Select LoRa mode (must be done in Sleep)
5. Enter Standby mode
6. Configure: frequency, BW, SF, CR, CRC, sync word, FIFO bases
7. Enter TX or RX when ready
```

> All config registers must be written in Sleep or Standby mode only.

---

## Driver Configuration

This driver is configured for:

| Parameter | Value |
|-----------|-------|
| Frequency | 915 MHz |
| Spreading Factor | SF7 |
| Bandwidth | 500 kHz |
| Coding Rate | 4/5 |
| Header Mode | Explicit |
| CRC | Enabled |
| Low Data Rate Optimize | Off |

---

## Driver API

### `Lora_init()`
Initializes the radio. Must be called after HAL and SPI are ready.

```cpp
bool Lora_init();
```

- Resets to Sleep, verifies chip version, switches to LoRa mode
- Configures frequency, BW, SF, CR, CRC
- Returns `false` if chip not detected

---

### `send()`
Transmits a packet. Blocks until TxDone or timeout.

```cpp
bool send(const uint8_t *data, uint8_t len);
```

**TX sequence:**
```
1. Enter Standby
2. Clear IRQ flags
3. Set FIFO pointer to TX base address
4. Write payload length
5. Write data into FIFO
6. Enter TX mode
7. Poll TxDone flag (5 second timeout)
8. Clear TxDone flag
```

---

### `RX_cont()`
Listens continuously for a packet. Blocks until a valid packet is received or timeout.

```cpp
uint8_t RX_cont(uint8_t *buf, uint8_t buf_size);
```

Returns number of bytes received, or `0` on timeout or error.

**RX sequence:**
```
1. Enter Standby
2. Clear IRQ flags
3. Enter RxContinuous mode
4. Poll RxDone flag (5 second timeout)
5. If CRC error: clear flags, keep waiting
6. Read RegFifoRxCurrentAddr → set RegFifoAddrPtr
7. Read RegRxNbBytes → packet length
8. Read FIFO that many times into buf
9. Clear IRQ flags
```

---

## RX Modes

- **RxSingle** - listens for one packet then returns to Standby automatically
- **RxContinuous** - keeps listening indefinitely; radio stays in RX until you change the mode

---

## CAD (Channel Activity Detection)

CAD checks whether there is LoRa activity on the channel without fully receiving a packet. Useful for low-power applications:

```
Enter CAD mode
    ↓
CadDone IRQ fires
    ↓
Check CadDetected flag
    ↙         ↘
detected     not detected
    ↓              ↓
enter RX       go back to Sleep
```

---

## DIO Pin Mapping

DIO pins can be wired to STM32 GPIO inputs for interrupt-driven operation (not required for this polling driver).

```
Mapping    DIO5         DIO4           DIO3               DIO2                 DIO1                 DIO0
--------   ----------   ------------   ----------------   ------------------   ------------------   -----------
00         ModeReady    CadDetected    CadDone            FhssChangeChannel    RxTimeout            RxDone
01         ClkOut       PllLock        ValidHeader        FhssChangeChannel    FhssChangeChannel    TxDone
10         ClkOut       PllLock        PayloadCrcError    FhssChangeChannel    CadDetected          CadDone
11         -            -              -                  -                    -                    -
```

Configured via `REG_DIO_MAPPING1` [7:6] for DIO0.

---

## Power Amplifier

The PA boosts the RF signal before the antenna. The SX1276 has two output pins:

- **RFO** - up to +14 dBm, lower power consumption
- **PA_BOOST** - up to +20 dBm via `REG_PA_DAC`, higher current draw

> High power mode can brown out and damage the module. Use with caution.

---

## Notes

- `printf` is retargeted to UART1 via `_write` syscall override
- All multi-byte integers use little-endian byte order (native ARM)
- Use `#pragma pack(1)` or `__attribute__((packed))` on telemetry structs to prevent padding
- Config registers must only be written in Sleep or Standby mode
- IRQ flag registers use Write-1-to-Clear - writing `0xFF` clears all flags
- I have hardcoded to pinouts to make it simple for me, if you want to change it modify in Wrapper.c
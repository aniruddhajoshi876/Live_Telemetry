/*
 * sx1276.hpp
 *
 *  Created on: Apr 30, 2026
 *      Author: aniru
 */
#include "spi.h"
#include "SPIDevice.hpp"
#include "SPIDriver.h"

#ifndef INC_SX1276_HPP_
#define INC_SX1276_HPP_

/* =========================================================================
 * COMMON REGISTERS  (shared between FSK/OOK and LoRa modes)
 * ========================================================================= */

#define REG_FIFO                0x00  /* SPI gateway to the 256-byte TX/RX FIFO buffer */
#define REG_OP_MODE             0x01  /* Selects operating mode (Sleep/Standby/TX/RX) and modem type (LoRa vs FSK/OOK) */
#define REG_FRF_MSB             0x06  /* Bits [23:16] of the 24-bit RF carrier frequency word */
#define REG_FRF_MID             0x07  /* Bits [15:8]  of the 24-bit RF carrier frequency word */
#define REG_FRF_LSB             0x08  /* Bits [7:0]   of the 24-bit RF carrier frequency word; Frf = Fxosc / 2^19 * RegFrf */
#define REG_PA_CONFIG           0x09  /* Selects output pin (RFO or PA_BOOST) and sets output power level */
#define REG_PA_RAMP             0x0A  /* Controls PA ramp-up/down time to reduce spectral splatter */
#define REG_OCP                 0x0B  /* Over-current protection: enables it and sets the current limit */
#define REG_LNA                 0x0C  /* Low-noise amplifier gain and boost (HF pin) settings */
#define REG_DIO_MAPPING1        0x40  /* Assigns functions to DIO0-DIO3 pins (e.g. RxDone, TxDone) */
#define REG_DIO_MAPPING2        0x41  /* Assigns functions to DIO4-DIO5 pins and sets CLKOUT divider */
#define REG_VERSION             0x42  /* Read-only silicon revision ID; default 0x12 */
#define REG_TCXO                0x4B  /* Chooses between internal crystal or external TCXO clock source */
#define REG_PA_DAC              0x4D  /* Enables +20 dBm high-power mode on PA_BOOST pin */
#define REG_FORMER_TEMP         0x5B  /* Temperature snapshot saved during the last IQ calibration run */
#define REG_AGC_REF             0x61  /* Reference level used by the AGC to set its gain target */
#define REG_AGC_THRESH1         0x62  /* AGC gain-step threshold 1 (defines when gain steps trigger) */
#define REG_AGC_THRESH2         0x63  /* AGC gain-step threshold 2 */
#define REG_AGC_THRESH3         0x64  /* AGC gain-step threshold 3 */
#define REG_PLL                 0x70  /* PLL bandwidth selection (affects phase noise and lock time) */

/* =========================================================================
 * FSK / OOK MODE REGISTERS
 * ========================================================================= */

#define REG_BITRATE_MSB         0x02  /* Upper byte of the bit-rate divider; BitRate = Fxosc / RegBitrate */
#define REG_BITRATE_LSB         0x03  /* Lower byte of the bit-rate divider */
#define REG_FDEV_MSB            0x04  /* Upper 6 bits of FSK frequency deviation; Fdev = Fxosc / 2^19 * RegFdev */
#define REG_FDEV_LSB            0x05  /* Lower 8 bits of FSK frequency deviation */
#define REG_RX_CONFIG           0x0D  /* Controls AFC trigger, AGC on/off, and RX start source */
#define REG_RSSI_CONFIG         0x0E  /* Sets RSSI smoothing filter and offset correction */
#define REG_RSSI_COLLISION      0x0F  /* RSSI drop threshold used to detect a channel collision */
#define REG_RSSI_THRESH         0x10  /* RSSI level that triggers the RSSI interrupt */
#define REG_RSSI_VALUE          0x11  /* Instantaneous RSSI reading in dBm (read-only) */
#define REG_RX_BW               0x12  /* Main channel receive filter bandwidth */
#define REG_AFC_BW              0x13  /* Receive filter bandwidth used during AFC acquisition */
#define REG_OOK_PEAK            0x14  /* OOK demodulator mode and peak-detector decay rate */
#define REG_OOK_FIX             0x15  /* Fixed threshold level for OOK fixed-threshold mode */
#define REG_OOK_AVG             0x16  /* Controls averaging filter for OOK average-threshold mode */
#define REG_AFC_FEI             0x1A  /* Triggers AFC/FEI operations and shows AFC on/off status */
#define REG_AFC_MSB             0x1B  /* Upper byte of the AFC frequency correction word */
#define REG_AFC_LSB             0x1C  /* Lower byte of the AFC frequency correction word */
#define REG_FEI_MSB             0x1D  /* Upper byte of the measured frequency error (read-only) */
#define REG_FEI_LSB             0x1E  /* Lower byte of the measured frequency error (read-only) */
#define REG_PREAMBLE_DETECT     0x1F  /* Enables preamble detector and sets qualifying preamble size */
#define REG_RX_TIMEOUT1         0x20  /* Time from RX start before RSSI must be detected */
#define REG_RX_TIMEOUT2         0x21  /* Time from RSSI detection before a sync address must be found */
#define REG_RX_TIMEOUT3         0x22  /* Time from sync detection before signal must be acquired */
#define REG_RX_DELAY            0x23  /* Dead time inserted between two consecutive RX windows */
#define REG_OSC                 0x24  /* RC oscillator calibration trigger and CLKOUT pin divider */
#define REG_PREAMBLE_MSB_FSK    0x25  /* Upper byte of the preamble length to transmit */
#define REG_PREAMBLE_LSB_FSK    0x26  /* Lower byte of the preamble length to transmit */
#define REG_SYNC_CONFIG         0x27  /* Sync word on/off, size, and preamble polarity (0xAA or 0x55) */
#define REG_SYNC_VALUE1         0x28  /* Sync word byte 1 (first byte sent/expected on air) */
#define REG_SYNC_VALUE2         0x29  /* Sync word byte 2 */
#define REG_SYNC_VALUE3         0x2A  /* Sync word byte 3 */
#define REG_SYNC_VALUE4         0x2B  /* Sync word byte 4 */
#define REG_SYNC_VALUE5         0x2C  /* Sync word byte 5 */
#define REG_SYNC_VALUE6         0x2D  /* Sync word byte 6 */
#define REG_SYNC_VALUE7         0x2E  /* Sync word byte 7 */
#define REG_SYNC_VALUE8         0x2F  /* Sync word byte 8 */
#define REG_PACKET_CONFIG1      0x30  /* Packet format (fixed/variable), DC-free encoding, CRC, address filtering */
#define REG_PACKET_CONFIG2      0x31  /* Enables packet mode vs continuous mode; io-homecontrol compatibility */
#define REG_PAYLOAD_LENGTH_FSK  0x32  /* Expected payload length in fixed-length packet mode */
#define REG_NODE_ADRS           0x33  /* Node address matched against received destination address */
#define REG_BROADCAST_ADRS      0x34  /* Broadcast address accepted by all nodes regardless of node address */
#define REG_FIFO_THRESH         0x35  /* TX starts when FIFO exceeds this level; also sets FifoLevel flag */
#define REG_SEQ_CONFIG1         0x36  /* Auto-sequencer enable, from-start state, and idle mode */
#define REG_SEQ_CONFIG2         0x37  /* Sequencer transition conditions from Receive and Transmit states */
#define REG_TIMER_RESOL         0x38  /* Resolution (tick period) for Timer1 and Timer2 */
#define REG_TIMER1_COEF         0x39  /* Timer1 multiplier; timeout = resolution x coefficient */
#define REG_TIMER2_COEF         0x3A  /* Timer2 multiplier; timeout = resolution x coefficient */
#define REG_IMAGE_CAL           0x3B  /* Triggers automatic image and RSSI calibration; shows cal status */
#define REG_TEMP                0x3C  /* Raw temperature sensor value (read-only; requires calibration) */
#define REG_LOW_BAT             0x3D  /* Low-battery detector enable and voltage trip threshold */
#define REG_IRQ_FLAGS1          0x3E  /* Interrupt flags: ModeReady, RxReady, TxReady, PllLock, etc. */
#define REG_IRQ_FLAGS2          0x3F  /* Interrupt flags: FifoFull, FifoEmpty, FifoLevel, PacketSent, etc. */
#define REG_BITRATE_FRAC        0x5D  /* Fractional part of bit-rate divider for fine rate adjustment */

/* =========================================================================
 * LoRa MODE REGISTERS
 * ========================================================================= */

#define REG_FIFO_ADDR_PTR        0x0D  /* SPI read/write pointer into the FIFO; set before each access */
#define REG_FIFO_TX_BASE_ADDR    0x0E  /* Start of the TX region inside the FIFO (default 0x80) */
#define REG_FIFO_RX_BASE_ADDR    0x0F  /* Start of the RX region inside the FIFO (default 0x00) */
#define REG_FIFO_RX_CURRENT_ADDR 0x10  /* FIFO address where the last received packet begins (read-only) */
#define REG_IRQ_FLAGS_MASK       0x11  /* Masks individual IRQ bits so they don't assert on the DIO pin */
#define REG_IRQ_FLAGS            0x12  /* IRQ status flags; write 1 to a bit to clear it */
#define REG_RX_NB_BYTES          0x13  /* Number of payload bytes in the last received packet (read-only) */
#define REG_RX_HEADER_CNT_MSB    0x14  /* Upper byte of valid header count since last RX mode entry */
#define REG_RX_HEADER_CNT_LSB    0x15  /* Lower byte of valid header count since last RX mode entry */
#define REG_RX_PACKET_CNT_MSB    0x16  /* Upper byte of valid packet count since last RX mode entry */
#define REG_RX_PACKET_CNT_LSB    0x17  /* Lower byte of valid packet count since last RX mode entry */
#define REG_MODEM_STAT           0x18  /* Live modem status: signal detected, synced, header valid (read-only) */
#define REG_PKT_SNR_VALUE        0x19  /* SNR of last packet in 0.25 dB steps; negative = below noise floor */
#define REG_PKT_RSSI_VALUE       0x1A  /* RSSI of last received packet (read-only) */
#define REG_RSSI_VALUE_LORA      0x1B  /* Current instantaneous RSSI during reception (read-only) */
#define REG_HOP_CHANNEL          0x1C  /* Current FHSS channel and CRC-on-payload flag from last header */
#define REG_MODEM_CONFIG1        0x1D  /* Bandwidth, coding rate, and implicit/explicit header mode */
#define REG_MODEM_CONFIG2        0x1E  /* Spreading factor, TX continuous mode, CRC enable, symbol timeout MSB */
#define REG_SYMB_TIMEOUT_LSB     0x1F  /* Lower 8 bits of the RX symbol timeout (combined with CONFIG2 MSB) */
#define REG_PREAMBLE_MSB_LORA    0x20  /* Upper byte of preamble symbol count to transmit */
#define REG_PREAMBLE_LSB_LORA    0x21  /* Lower byte of preamble symbol count to transmit */
#define REG_PAYLOAD_LENGTH_LORA  0x22  /* Payload byte count for implicit header mode; used in explicit mode too */
#define REG_MAX_PAYLOAD_LENGTH   0x23  /* RX payload will be truncated if it exceeds this length */
#define REG_HOP_PERIOD           0x24  /* Symbol periods between FHSS hops; 0 disables FHSS */
#define REG_FIFO_RX_BYTE_ADDR    0x25  /* FIFO address of the last byte written during current reception */
#define REG_MODEM_CONFIG3        0x26  /* Low data-rate optimization and AGC auto-on settings */
#define REG_FEI_MSB_LORA         0x28  /* Bits [19:16] of the estimated RF frequency error (read-only) */
#define REG_FEI_MID_LORA         0x29  /* Bits [15:8]  of the estimated RF frequency error (read-only) */
#define REG_FEI_LSB_LORA         0x2A  /* Bits [7:0]   of the estimated RF frequency error (read-only) */
#define REG_RSSI_WIDEBAND        0x2C  /* Broadband RSSI used internally for random number generation */
#define REG_DETECT_OPTIMIZE      0x31  /* Set to 0x05 for SF6, 0x03 for SF7-SF12 (detection optimizer) */
#define REG_INVERT_IQ            0x33  /* Inverts I/Q channels; used to differentiate uplink/downlink */
#define REG_DETECTION_THRESHOLD  0x37  /* Detection threshold: 0x0A for SF7-SF12, 0x0C for SF6 */
#define REG_SYNC_WORD            0x39  /* Network identifier byte; 0x12 = private, 0x34 = LoRaWAN */

/* =========================================================================
 * REGISTER DEFAULT VALUES  (power-on reset)
 * ========================================================================= */

#define DEF_OP_MODE             0x01  /* Standby mode, FSK modulation selected */
#define DEF_BITRATE_MSB         0x1A  /* MSB of default 4.8 kbps bit rate */
#define DEF_BITRATE_LSB         0x0B  /* LSB of default 4.8 kbps bit rate */
#define DEF_FDEV_MSB            0x00  /* MSB of default 5 kHz frequency deviation */
#define DEF_FDEV_LSB            0x52  /* LSB of default 5 kHz frequency deviation */
#define DEF_FRF_MSB             0x6C  /* MSB of default 434 MHz carrier frequency */
#define DEF_FRF_MID             0x80  /* MID byte of default 434 MHz carrier frequency */
#define DEF_FRF_LSB             0x00  /* LSB of default 434 MHz carrier frequency */
#define DEF_PA_CONFIG           0x4F  /* RFO pin selected, Pmax = +13.8 dBm, Pout = +11.4 dBm */
#define DEF_PA_RAMP             0x09  /* 40 us PA ramp time */
#define DEF_OCP                 0x2B  /* OCP enabled, 100 mA limit */
#define DEF_LNA                 0x20  /* Maximum LNA gain, no boost */
#define DEF_SYNC_WORD           0x12  /* Private LoRa network (use 0x34 for LoRaWAN) */
#define DEF_PA_DAC              0x84  /* Normal PA operation (+17 dBm max on PA_BOOST) */
#define DEF_TCXO                0x09  /* Crystal oscillator input selected */
#define DEF_VERSION             0x12  /* Silicon revision; read this at init to verify SPI comms */

/* =========================================================================
 * RegOpMode (0x01) - bit field values
 * ========================================================================= */

#define LONG_RANGE_MODE         (1 << 7)  /* Set to enable LoRa modem; clear for FSK/OOK */
#define MODULATION_TYPE_FSK     (0 << 5)  /* FSK modulation (FSK/OOK mode only) */
#define MODULATION_TYPE_OOK     (1 << 5)  /* OOK modulation (FSK/OOK mode only) */
#define LOW_FREQ_MODE_ON        (1 << 3)  /* Enables low-frequency (sub-860 MHz) register set */
#define MODE_SLEEP              0x00      /* Lowest power; FIFO is lost */
#define MODE_STDBY              0x01      /* Crystal on, ready to configure; FIFO retained */
#define MODE_FSTX               0x02      /* Frequency synthesizer powered up for TX */
#define MODE_TX                 0x03      /* Transmitting packet */
#define MODE_FSRX               0x04      /* Frequency synthesizer powered up for RX */
#define MODE_RXCONTINUOUS       0x05      /* Continuously listening for packets */
#define MODE_RXSINGLE           0x06      /* Listen for one packet then return to standby */
#define MODE_CAD                0x07      /* Channel Activity Detection: checks for LoRa preamble on air */

/* =========================================================================
 * RegIrqFlags (LoRa 0x12) - bit masks
 * ========================================================================= */

#define IRQ_RX_TIMEOUT          (1 << 7)  /* No packet received within the symbol timeout window */
#define IRQ_RX_DONE             (1 << 6)  /* Packet reception complete; read FIFO now */
#define IRQ_PAYLOAD_CRC_ERROR   (1 << 5)  /* CRC mismatch on received payload */
#define IRQ_VALID_HEADER        (1 << 4)  /* LoRa header received and parsed successfully */
#define IRQ_TX_DONE             (1 << 3)  /* Packet transmission finished */
#define IRQ_CAD_DONE            (1 << 2)  /* Channel Activity Detection cycle complete */
#define IRQ_FHSS_CHANGE_CHANNEL (1 << 1)  /* FHSS hop occurred; update frequency now */
#define IRQ_CAD_DETECTED        (1 << 0)  /* LoRa preamble detected during CAD */

/* =========================================================================
 * RegPaConfig (0x09) - bit field masks
 * ========================================================================= */

#define PA_SELECT_BOOST         (1 << 7)  /* Route PA output through PA_BOOST pin (up to +20 dBm with PA_DAC) */
#define PA_SELECT_RFO           (0 << 7)  /* Route PA output through RFO pin (up to +14 dBm) */
#define MAX_POWER_MASK          0x70      /* Bits [6:4]: sets Pmax = 10.8 + 0.6*MaxPower dBm */
#define OUTPUT_POWER_MASK       0x0F      /* Bits [3:0]: Pout = Pmax - (15 - OutputPower) on RFO pin */

#define LORA_MAX_TX_PAYLOAD	64

class sx1276{
public:
	sx1276(SPI_HandleTypeDef* hspi, GPIO_TypeDef *nssPort, uint16_t nssPin,
		   GPIO_TypeDef *resetPort, uint16_t resetPin, GPIO_TypeDef *dio0Port, uint16_t dio0Pin);
	
	bool Lora_init();

	uint8_t readreg(uint8_t address);
	bool writereg(uint8_t address, uint8_t data);
	bool writebuffer(uint8_t address, const uint8_t* buffer, uint8_t len);
	bool send(const uint8_t* data, uint8_t len);

	uint8_t RX_cont(uint8_t* buf, uint8_t buf_size);


private:
	SPIDriver _spiDriver;
	IOPin CS_pin;
	SPIDevice _spiDevice;  
    IOPin     resetPin;
    IOPin     dio0Pin;

};
#endif /* INC_SX1276_HPP_ */

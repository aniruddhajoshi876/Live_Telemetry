/*
 * sx1276.cpp
 *
 *  Created on: Apr 30, 2026
 *      Author: aniru
 */

#include "sx1276.hpp"
#include <cstdio>
#include <cstring>

sx1276::sx1276(SPI_HandleTypeDef* hspi, GPIO_TypeDef *nssPort, uint16_t nssPin,
		GPIO_TypeDef *resetPort, uint16_t resetPin, GPIO_TypeDef *dio0Port, uint16_t dio0Pin):
		_spiDriver(hspi),CS_pin(nssPort, nssPin), _spiDevice(_spiDriver, CS_pin), resetPin(resetPort, resetPin), dio0Pin(dio0Port, dio0Pin)
{
	}

bool sx1276::Lora_init() {

	uint8_t version = readreg(REG_VERSION); //verify if sx1276 is detected and spi is working
	if (version != DEF_VERSION) {
		printf("SX1276 not found! Got 0x%02X\r\n", version);
		return false;
	}

    // Must be in sleep to switch to LoRa mode
    writereg(REG_OP_MODE, MODE_SLEEP);
    HAL_Delay(10);




    printf("Lora Module detected!\r\n");



    // Switch to LoRa mode
    writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_SLEEP);
    HAL_Delay(10);

    // Set frequency to 915 MHz
    uint32_t frf = (uint32_t)(433e6 / 61.035);  // Fstep = Fxosc/2^19 = 61.035 Hz
    writereg(REG_FRF_MSB, (frf >> 16) & 0xFF);
    writereg(REG_FRF_MID, (frf >> 8)  & 0xFF);
    writereg(REG_FRF_LSB, (frf >> 0)  & 0xFF);

    // REG_MODEM_CONFIG1: BW=500kHz (1001), CodingRate=4/5 (001), ExplicitHeader (0)
    writereg(REG_MODEM_CONFIG1, (0b1001 << 4) | (0b001 << 1) | 0);

    // REG_MODEM_CONFIG2: SF=7 (0111), normal mode, CRC on, timeout MSB=0
    writereg(REG_MODEM_CONFIG2, (7 << 4) | (1 << 2));

    // Back to standby
    writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_STDBY);
    printf("radio initialized\r\n");
    return true;
}
uint8_t sx1276::readreg(uint8_t address){
	uint8_t tx[2];
	uint8_t rx[2];
	tx[0] = address & 0x7F;
	tx[1] = 0x00;
	_spiDevice.transmit_Receive(tx, sizeof(tx), rx, 2);
	return rx[1];
}

bool sx1276::writereg(uint8_t address, uint8_t data){
	uint8_t tx[2];
	tx[0] = address | 0x80;
	tx[1] = data;

	return _spiDevice.transmit(tx, sizeof(tx));
}

bool sx1276::writebuffer(uint8_t address, const uint8_t *buffer, uint8_t len)
{
    if (buffer == nullptr || len == 0) return false;

    uint8_t tx[len + 1];
    tx[0] = address | 0x80;
    memcpy(&tx[1], buffer, len);

    return _spiDevice.transmit(tx, len + 1);
}

bool sx1276::send(const uint8_t *data, uint8_t len)
{
    if (len == 0 || len > LORA_MAX_TX_PAYLOAD) {
        return false;
    }

    // enter Standby mode
    writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_STDBY);

    // Clear old IRQ flags
    writereg(REG_IRQ_FLAGS, 0xFF);

    // Write Data FIFO
    writereg(REG_FIFO_ADDR_PTR, readreg(REG_FIFO_TX_BASE_ADDR)); //find where the region tx is written in
    writereg(REG_PAYLOAD_LENGTH_LORA, len); //write data size
    writebuffer(REG_FIFO, data, len); //write data

    // Mode Request TX
    writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_TX); //begins the sending process

    // Wait TxDone, polling version
    uint32_t start = HAL_GetTick();
    while (!(readreg(REG_IRQ_FLAGS) & IRQ_TX_DONE)) {
        if (HAL_GetTick() - start > 5000) {
            printf("TX timeout\r\n");
            writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_STDBY);
            return false;
        }
    }

    // Clear TxDone flag
    writereg(REG_IRQ_FLAGS, IRQ_TX_DONE);

    return true;
}

uint8_t sx1276::RX_cont(uint8_t* buf, uint8_t buf_size){

	// enter Standby mode, small config step before entering rx mode
	writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_STDBY);

	//Clear any stale IRQ flags before we start listening
	writereg(REG_IRQ_FLAGS, 0xFF);

	//rx init + enter rx_continous mode
	writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_RXCONTINUOUS);

	//	RegIrqFlags (0x12) looks like this:
	//
	//	bit 7 → RxTimeout
	//	bit 6 → RxDone
	//	bit 5 → PayloadCrcError
	//	bit 4 → ValidHeader
	//	bit 3 → TxDone
	//	bit 2 → CadDone
	//	bit 1 → FhssChangeChannel
	//	bit 0 → CadDetected



	//Polling loop
	uint32_t start = HAL_GetTick();
	while (1) {
		if (HAL_GetTick() - start > 5000) {
			printf("RX timeout\r\n");
			writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_STDBY); //reenter standby mode to save power
			return 0;
		}
		uint8_t flags = readreg(REG_IRQ_FLAGS);

		// Did we receive a packet?
		if (flags & IRQ_RX_DONE) {

			// Did it fail the CRC check?
			if (flags & IRQ_PAYLOAD_CRC_ERROR) {
				printf("Packet received, but CRC error! Ignoring...\r\n");

				// Clear the flags so the radio can keep listening for the next one, special interupt registers use Write-1-to-Clear
				writereg(REG_IRQ_FLAGS, IRQ_RX_DONE | IRQ_PAYLOAD_CRC_ERROR);
				continue; // Stay in the loop and wait for a good packet
			}

			// If we made it here, the packet is good
			printf("Packet received, no crc error\r\n");
			break;
		}
	}

	uint8_t rx_start = readreg(REG_FIFO_RX_CURRENT_ADDR); //find where the start of the packet was written into

	writereg(REG_FIFO_ADDR_PTR, rx_start); // write start of packet into this pointer to know where to begin reading

	uint8_t packet_len = readreg(REG_RX_NB_BYTES); //found out how long packet is

	//MUST ensure 'buf' is large enough to hold 'packet_len' bytes
	if (packet_len > buf_size) {
		printf("Buffer too small\r\n");
		writereg(REG_OP_MODE, LONG_RANGE_MODE | MODE_STDBY); //reenter standby mode to save power
		return 0;
	}

	for (int i=0; i < packet_len; i++){

		buf[i] = readreg(REG_FIFO);
	}

	//Clear the IRQ flags so we don't instantly trigger again next time
	writereg(REG_IRQ_FLAGS, 0xFF);

	return packet_len;
}

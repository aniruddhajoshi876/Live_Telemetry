/*
 * SPIDriver.h
 *
 *  Created on: Feb 13, 2025
 *      Author: mariobouzakhm
 */

#ifndef INC_SPIDRIVER_H_
#define INC_SPIDRIVER_H_

#include "spi.h"

class SPIDriver {
	private:
		SPI_HandleTypeDef* spiInstance;
		uint32_t defaultTimeout;

	public:
		SPIDriver(SPI_HandleTypeDef* spiInstance, uint32_t defaultTimeout = 100);

		bool transmit(uint8_t* data, uint16_t size);
		bool transmit_Timeout(uint8_t* data, uint16_t size, uint32_t timeout);
		bool receive(uint8_t* rData, uint16_t size);
		bool receive_Timeout(uint8_t* rData, uint16_t size, uint32_t timeout);
		bool transmit_Receive(uint8_t* tData, uint16_t tSize, uint8_t *rData, uint16_t rSize);
		bool transmit_Receive_Timeout(uint8_t* tData, uint16_t tSize, uint8_t *rData, uint16_t rSize, uint32_t timeout);
};

#endif /* INC_SPIDRIVER_H_ */

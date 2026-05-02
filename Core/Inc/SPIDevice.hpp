/*
 * SPIDevice.hpp
 *
 *  Created on: Feb 14, 2025
 *      Author: mariobouzakhm
 */

#ifndef INC_SPIDEVICE_HPP_
#define INC_SPIDEVICE_HPP_

#include "IOPin.hpp"
#include "SPIDriver.h"

class SPIDevice {
	private:
		SPIDriver& spiDriver;
		IOPin& spiCS;
	public:
		SPIDevice(SPIDriver& spiDriver, IOPin& spiCS);
		bool transmit(uint8_t* data, uint16_t size);
		bool receive(uint8_t* rData, uint16_t size);
		bool transmit_Receive(uint8_t* tData, uint16_t tSize, uint8_t *rData, uint16_t rSize);
};

#endif /* INC_SPIDEVICE_HPP_ */

/*
 * SPIDevice.cpp
 *
 *  Created on: Feb 15, 2025
 *      Author: mariobouzakhm
 */

#include "SPIDevice.hpp"

SPIDevice::SPIDevice(SPIDriver& spiDriver, IOPin& spiCS)
	: spiDriver(spiDriver), spiCS(spiCS)
{
	spiCS.setPin(GPIO_PIN_SET);
}

bool SPIDevice::transmit(uint8_t *data, uint16_t size) {
	spiCS.setPin(GPIO_PIN_RESET);
	bool result = spiDriver.transmit(data, size);
	spiCS.setPin(GPIO_PIN_SET);

	return result;
}

bool SPIDevice::receive(uint8_t *rData, uint16_t size) {
	spiCS.setPin(GPIO_PIN_RESET);
	bool result = spiDriver.receive(rData, size);
	spiCS.setPin(GPIO_PIN_SET);

	return result;
}

bool SPIDevice::transmit_Receive(uint8_t* tData, uint16_t tSize, uint8_t *rData, uint16_t rSize) {
	spiCS.setPin(GPIO_PIN_RESET);
	bool result = spiDriver.transmit_Receive(tData, tSize, rData, rSize);
	spiCS.setPin(GPIO_PIN_SET);

	return result;
}





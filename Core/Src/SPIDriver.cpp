/*
 * SPIDriver.cpp
 *
 *  Created on: Feb 13, 2025
 *      Author: mariobouzakhm
 */

#include "SPIDriver.h"

SPIDriver::SPIDriver(SPI_HandleTypeDef* spiInstance, uint32_t defaultTimeout)
	: spiInstance(spiInstance)
{
	this->defaultTimeout = defaultTimeout;
}

bool SPIDriver::transmit(uint8_t* data, uint16_t size) {
	HAL_StatusTypeDef result = HAL_SPI_Transmit(spiInstance, data, size, defaultTimeout);

	return (result == HAL_OK);
}

bool SPIDriver::transmit_Timeout(uint8_t* data, uint16_t size, uint32_t timeout) {
	HAL_StatusTypeDef result = HAL_SPI_Transmit(spiInstance, data, size, timeout);

	return (result == HAL_OK);
}
bool SPIDriver::receive(uint8_t* rData, uint16_t size) {
	HAL_StatusTypeDef result = HAL_SPI_Receive(spiInstance, rData, size, defaultTimeout);

	return (result == HAL_OK);
}
bool SPIDriver::receive_Timeout(uint8_t* rData, uint16_t size, uint32_t timeout) {
	HAL_StatusTypeDef result = HAL_SPI_Receive(spiInstance, rData, size, timeout);

	return (result == HAL_OK);
}
bool SPIDriver::transmit_Receive(uint8_t* tData, uint16_t tSize, uint8_t *rData, uint16_t rSize) {

	if (tSize != rSize) return false;

	HAL_StatusTypeDef result = HAL_SPI_TransmitReceive(spiInstance, tData, rData, tSize, defaultTimeout);
	return (result == HAL_OK);
	//HAL_StatusTypeDef result = HAL_SPI_Transmit(spiInstance, tData, tSize, defaultTimeout);
	//HAL_StatusTypeDef result2 = HAL_SPI_Receive(spiInstance, rData, rSize, defaultTimeout);
	//return (result == HAL_OK) && (result2 == HAL_OK);
}
bool SPIDriver::transmit_Receive_Timeout(uint8_t* tData, uint16_t tSize, uint8_t *rData, uint16_t rSize, uint32_t timeout) {
	HAL_StatusTypeDef result = HAL_SPI_Transmit(spiInstance, tData, tSize, timeout);
	HAL_StatusTypeDef result2 = HAL_SPI_Receive(spiInstance, rData, rSize, timeout);

	return (result == HAL_OK) & (result2 == HAL_OK);
}







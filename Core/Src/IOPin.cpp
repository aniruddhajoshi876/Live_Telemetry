/*
 * IOPin.cpp
 *
 *  Created on: Feb 13, 2025
 *      Author: mariobouzakhm
 */

#include "IOPin.hpp"

IOPin::IOPin() {
}

IOPin::IOPin(GPIO_TypeDef * gpio_base, uint16_t gpio_pin) {
	this->gpio_base = gpio_base;
	this->gpio_pin = gpio_pin;
}

uint8_t IOPin::readPin() {
	return HAL_GPIO_ReadPin(gpio_base, gpio_pin);
}
void IOPin::setPin(GPIO_PinState value) {
	HAL_GPIO_WritePin(this->gpio_base, this->gpio_pin, value);
}
void IOPin::togglePin() {
	HAL_GPIO_TogglePin(this->gpio_base, this->gpio_pin);
}


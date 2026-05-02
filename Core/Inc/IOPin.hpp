/*
 * IOPin.hpp
 *
 *  Created on: Feb 13, 2025
 *      Author: mariobouzakhm
 */

#ifndef INC_IOPIN_HPP_
#define INC_IOPIN_HPP_

#include "gpio.h"

class IOPin {
private:
	GPIO_TypeDef *gpio_base;
	uint16_t gpio_pin;

public:
	IOPin();
	IOPin(GPIO_TypeDef * gpio_base, uint16_t gpio_pin);

	uint8_t readPin();
	void setPin(GPIO_PinState value);
	void togglePin();
};




#endif /* INC_IOPIN_HPP_ */

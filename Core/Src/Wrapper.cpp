/*
 * Wrapper.cpp
 *
 *  Created on: May 2, 2026
 *      Author: aniru
 */

#include "Wrapper.h"
#include "BoardManager.hpp"
#include "spi.h"
#include "gpio.h"

extern SPI_HandleTypeDef hspi1;

void* bm_init(){
	sx1276* radio = new sx1276(&hspi1, GPIOB, GPIO_PIN_15, GPIOB, GPIO_PIN_13, GPIOB, GPIO_PIN_14);
	return new BoardManager(radio);
}

bool bm_radio_init(void* bm){
	return ((BoardManager*) bm)->bm_radio_init();
}

void bm_send(void* bm, const uint8_t* data, uint8_t len){
	((BoardManager*) bm)->bm_send_packet(data, len);
}

void bm_receive(void* bm, uint8_t* buf, uint8_t size){
	((BoardManager*) bm)->bm_receive_packet(buf, size);
}






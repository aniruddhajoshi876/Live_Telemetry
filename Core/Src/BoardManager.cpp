/*
 * BoardManager.cpp
 *
 *  Created on: May 2, 2026
 *      Author: aniru
 */

#include "BoardManager.hpp"
#include <cstdio>

BoardManager::BoardManager(sx1276* radio): radio(radio){}

bool BoardManager::bm_radio_init(){
	return radio->Lora_init();
}

bool BoardManager::bm_send_packet(const uint8_t* data, uint8_t len){
	return radio->send(data, len);
}

uint8_t BoardManager::bm_receive_packet(uint8_t* buf, uint8_t size){
	uint8_t code = radio->RX_cont(buf, size);
	if (!code){
		printf("Packet Discarded\r\n");
		return 0;
	}
	printf("Packet received\r\n");
	printf("%.*s\r\n", size, buf);
	return code; //just returns length of packet received, make sure to pass in a buffer enough, max: 64 bytes needed
}






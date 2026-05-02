/*
 * BoardManager.hpp
 *
 *  Created on: May 2, 2026
 *      Author: aniru
 */

#ifndef INC_BOARDMANAGER_HPP_
#define INC_BOARDMANAGER_HPP_

#include "sx1276.hpp"

class BoardManager{
public:
	BoardManager(sx1276* radio);
	bool bm_radio_init();
	bool bm_send_packet(const uint8_t* data, uint8_t len);
	uint8_t bm_receive_packet(uint8_t* buf, uint8_t buf_size);
private:
	sx1276* radio;
};



#endif /* INC_BOARDMANAGER_HPP_ */

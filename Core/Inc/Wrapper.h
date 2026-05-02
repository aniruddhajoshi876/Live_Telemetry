/*
 * Wrapper.h
 *
 *  Created on: May 2, 2026
 *      Author: aniru
 */

#ifndef INC_WRAPPER_H_
#define INC_WRAPPER_H_
#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

void* bm_init();
bool bm_radio_init(void* bm);
void bm_send(void* bm, const uint8_t* data, uint8_t len);
void bm_receive(void* bm, uint8_t* buf, uint8_t size);

#ifdef __cplusplus
}
#endif



#endif /* INC_WRAPPER_H_ */

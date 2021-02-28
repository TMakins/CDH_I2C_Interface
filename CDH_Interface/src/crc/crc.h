/*
 * crc.h
 *
 * Created: 29/01/2021 17:25:06
 *  Author: Toby
 */ 


#ifndef CRC_H_
#define CRC_H_

#include <stdint.h>

uint16_t generate_crc(uint8_t *data, uint8_t len);


#endif /* CRC_H_ */
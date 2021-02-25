/*
 * heater.h
 *
 * Created: 29/01/2021 21:23:44
 *  Author: Toby
 */ 


#ifndef HEATER_H_
#define HEATER_H_

#include <stdint.h>

typedef enum save_reason {
	WATCHDOG = 0,
	NO_CONNECTION,
} save_reason_t;

class Heater
{
	public:
	Heater();
	void init();
	void process_rx_packet(uint8_t* data);
	uint8_t* prepare_tx_packet();
	uint8_t* get_i2c_regs();
	uint8_t get_i2c_regs_len();
	void disconnected();
	void save_to_eeprom(save_reason_t reason);
	
	uint8_t _connected;
};

extern Heater heater;

#endif /* HEATER_H_ */
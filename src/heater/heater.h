/*
 * heater.h
 *
 * Created: 29/01/2021 21:23:44
 *  Author: Toby
 */ 


#ifndef HEATER_H_
#define HEATER_H_

#include <stdint.h>

class Heater
{
	public:
	Heater();
	void init();
	void process_rx_packet(uint8_t* data);
	uint8_t* prepare_tx_packet();
	uint8_t* get_i2c_regs();
	void disconnected();
	
	uint8_t _connected;
};

extern Heater heater;

#endif /* HEATER_H_ */
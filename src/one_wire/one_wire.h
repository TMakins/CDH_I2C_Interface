/*
 * one_wire.h
 *
 * Created: 29/01/2021 17:50:56
 *  Author: Toby
 */ 


#ifndef ONE_WIRE_H_
#define ONE_WIRE_H_

#include <stdint.h>

class OneWire
{
	public:
	OneWire();
	void init(uint32_t baud);
	uint8_t available();	
	void read(uint8_t *buff, uint8_t len);
	void write(uint8_t *buff, uint8_t len);
	void clear_rx();
	
	private:
	void _tx_reset();
	bool _tx_ready();
	void _rx_disable();
	void _rx_enable();
	bool _tx_complete();
};



#endif /* ONE_WIRE_H_ */
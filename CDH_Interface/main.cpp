/*
 * CDH_Interface_Mega4809.cpp
 *
 * Created: 29/01/2021 16:50:34
 * Author : Toby
 */

#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include <system/system.h>
#include <crc/crc.h>
#include <one_wire/one_wire.h>
#include <timer/timer.h>
#include <heater/heater.h>
#include <i2c/i2c.h>

OneWire blue_wire;
I2C i2c;

#define LED_bp 1 //PA1
#define LED_bm (1<<LED_bp)

int main(void)
{
	system_init();
	
	//debug.init(115200);
	i2c_regs.regs.settings.debug = 0; // we're using the debug reg to count number of uart resets
	
	blue_wire = OneWire();
	blue_wire.init(25000);
	
	i2c.init(i2c_regs.data, sizeof(i2c_regs.data));
	
	// Enable interrupts
	sei();
		
	uint8_t data_in[24];
		
	uint32_t start_time = timer.millis();
	uint32_t last_read = 0;
	
	heater.init(); 	
	
    while (1) 
    {
	    wdt_reset();
	    if(timer.millis() - start_time >= 800) 
		{
		    start_time = timer.millis();
			uint8_t* data_out = heater.prepare_tx_packet();
			blue_wire.clear_rx();
		    blue_wire.write(data_out, 24);
	    }
		
		if(blue_wire.available() >= 24) 
		{
			last_read = timer.millis();
			blue_wire.read(data_in, 24);
			heater.process_rx_packet(data_in);
		}
		
		if((timer.millis() - last_read) >= 3000)
		{
			heater.disconnected();
			// Reset USART
			blue_wire.reset();
			last_read = timer.millis();
		}
    }
}


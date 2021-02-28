/*
 * system.cpp
 *
 * Created: 07/02/2021 10:21:31
 *  Author: Toby
 */ 

#include "system.h"
#include <avr/wdt.h>
#include <protected_io/protected_io.h>

void system_init()
{
	// Setup clock
	protected_write_io((void*)&(CLKCTRL.MCLKCTRLB), CCP_IOREG_gc, 0);
	
	// Setup watchdog
	wdt_enable(WDTO_2S);
	wdt_reset();
}
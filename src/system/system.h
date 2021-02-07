/*
 * system.h
 *
 * Created: 29/01/2021 17:42:01
 *  Author: Toby
 */ 


#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <avr/wdt.h>
#include <protected_io/protected_io.h>

void system_init()
{
	protected_write_io((void*)&(CLKCTRL.MCLKCTRLB), CCP_IOREG_gc, 0);
}

#endif /* SYSTEM_H_ */
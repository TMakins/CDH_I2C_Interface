#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

Timer::Timer()
{
	TCB0.CTRLB = TCB_CNTMODE_INT_gc;
	TCB0.CCMP = (F_CPU / 1000) - 1; // 1ms
	TCB0.INTCTRL |= TCB_CAPT_bm;
	TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc;
	
	// Enable and start
	TCB0.CTRLA |= TCB_ENABLE_bm;
	
	tick_ms = 0;
}

uint32_t Timer::millis()
{
	// Disable interrupts to prevent concurrent access
	uint8_t status = SREG;
	cli();
	
	// Re-enable
	SREG = status;
	
	return tick_ms;
}

//ISR for ms counter
ISR(TCB0_INT_vect) 
{
	timer.tick_ms++;
	
	TCB0.INTFLAGS = TCB_CAPT_bm;
}

// Create instance of timer (only one possible, so might as well create it now)
Timer timer = Timer();
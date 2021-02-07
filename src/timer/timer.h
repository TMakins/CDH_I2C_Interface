#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

class Timer
{
	public:
	Timer();
	uint32_t millis();
	
	volatile uint32_t tick_ms;
};

extern Timer timer;

#endif /* TIMER_H_ */
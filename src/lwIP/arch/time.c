#include <tice.h>
#include "../include/lwip/opt.h"

void timer_init(void) {
	timer_Control &= ~TIMER3_ENABLE;
	timer_3_Counter = 0;
	timer_Control |= TIMER3_ENABLE | TIMER3_32K | TIMER3_UP;
}

u32_t sys_now(void) {
	/* I don't think this timer is too important */
	//todo: make 32 bit, 1 ms
	return timer_3_Counter >> 5;
}
#ifndef __CLOCK_H
#define __CLOCK_H

extern volatile unsigned long tick_ms;

int mspm0_delay_ms(unsigned long num_ms);
int mspm0_get_clock_ms(unsigned long *count);

#endif

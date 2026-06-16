#include "ti_msp_dl_config.h"
#include "clock.h"
#include "hardware/delay.h"

volatile unsigned long tick_ms;

int mspm0_delay_ms(unsigned long num_ms)
{
    delay_ms(num_ms);  /* 直接用忙等待, 不依赖 tick_ms */
    return 0;
}

int mspm0_get_clock_ms(unsigned long *count)
{
    if (!count) return 1;
    count[0] = tick_ms;
    return 0;
}

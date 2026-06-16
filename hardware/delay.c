/**
 * @file  delay.c
 * @brief 毫秒/微秒级忙等待延时实现
 */

#include "delay.h"

/* 每次 delay_cycles 循环的近似开销 (约 3 cycles/loop on Cortex-M0+) */
#define DELAY_LOOP_OVERHEAD  3U

void delay_ms(uint32_t ms)
{
    delay_cycles((CPUCLK_FREQ / 1000UL) * ms);
}

void delay_us(uint32_t us)
{
    delay_cycles((CPUCLK_FREQ / 1000000UL) * us);
}

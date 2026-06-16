/**
 * @file  delay.h
 * @brief 毫秒/微秒级忙等待延时 — 基于 CPUCLK_FREQ
 *
 * 使用 DriverLib delay_cycles() 实现, 无需硬件定时器。
 * 精度依赖 CPUCLK_FREQ 宏 (由 SysConfig 生成, 80MHz)。
 */

#ifndef __DELAY_H
#define __DELAY_H

#include "ti_msp_dl_config.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 毫秒延时 (忙等待)
 * @param ms  毫秒数 (>0)
 */
void delay_ms(uint32_t ms);

/**
 * @brief 微秒延时 (忙等待)
 * @param us  微秒数 (>0, 最小约 3us)
 */
void delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* __DELAY_H */

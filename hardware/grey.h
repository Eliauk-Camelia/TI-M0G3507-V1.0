/**
 * @file  grey.h
 * @brief 感为灰度传感器 串行接口驱动
 *
 * 2 线串行协议 (CLK + DAT):
 *   CLK → MCU 输出, 下降沿后读 DAT
 *   DAT → MCU 输入 (外部上拉)
 *   8 个时钟周期返回 8 位数字量 (每 bit 对应一个探头)
 */

#ifndef __GREY_H
#define __GREY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化传感器 (串行模式)
 *
 * 发送特定时序使传感器进入串行数据模式。
 */
void grey_init(void);

/**
 * @brief 读取 8 路探头数字量
 * @return bit0~bit7 对应探头 1~8 (0=黑, 1=白, 需先校准)
 */
uint8_t grey_read_digital(void);

#ifdef __cplusplus
}
#endif

#endif /* __GREY_H */

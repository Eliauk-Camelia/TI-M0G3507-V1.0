/**
 * @file  gray.h
 * @brief 感为 (GW) 灰度传感器 I2C 驱动 API
 *
 * 传感器通过 I2C 通信 (默认地址 0x4C), 提供:
 *   - 数字量: 8 路探头黑白判断 (1 字节, 每 bit 对应一路)
 *   - 模拟量: 8 路探头灰度值 (8 字节, 0~255)
 */

#ifndef __GRAY_H
#define __GRAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化传感器, 发送 Ping 直到握手成功
 * @return 0=成功
 *
 * @note 上电后必须先调用此函数, 确保与传感器同步。
 *       若传感器无响应, 会无限循环等待。
 */
int gray_init(void);

/**
 * @brief 读取 8 路数字量 (0/1)
 * @param digital 输出: bit0~bit7 对应探头 1~8
 * @return 0=成功
 */
int gray_read_digital(uint8_t *digital);

/**
 * @brief 读取 8 路模拟量 (0~255)
 * @param analog 输出数组, 长度≥8, analog[0]~analog[7] 对应探头 1~8
 * @return 0=成功
 */
int gray_read_analog(uint8_t analog[8]);

#ifdef __cplusplus
}
#endif

#endif /* __GRAY_H */

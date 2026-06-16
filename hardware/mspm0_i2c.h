/**
 * @file  mspm0_i2c.h
 * @brief MSPM0 硬件 I2C 读写封装
 *
 * 基于 DL_I2C API, 用于 MPU6050 等 I2C 外设通信。
 */

#ifndef __MSPM0_I2C_H
#define __MSPM0_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief I2C 写操作 (寄存器地址 + 数据)
 * @param slave_addr  7 位从机地址 (不含 R/W 位)
 * @param reg_addr    寄存器地址
 * @param length      数据长度
 * @param data        数据指针
 * @return 0=成功, -1=超时
 */
int mspm0_i2c_write(uint8_t slave_addr,
                    uint8_t reg_addr,
                    uint8_t length,
                    const uint8_t *data);

/**
 * @brief I2C 读操作 (寄存器地址 → 读数据)
 * @param slave_addr  7 位从机地址
 * @param reg_addr    寄存器地址
 * @param length      读取长度
 * @param data        输出缓冲区
 * @return 0=成功, -1=超时/错误
 */
int mspm0_i2c_read(uint8_t slave_addr,
                   uint8_t reg_addr,
                   uint8_t length,
                   uint8_t *data);

/**
 * @brief SDA 解锁: 当 I2C 总线被从机拉死时, 发 9 个时钟脉冲恢复
 */
void mpu6050_i2c_sda_unlock(void);

#ifdef __cplusplus
}
#endif

#endif /* __MSPM0_I2C_H */

/**
 * @file  sw_i2c.h
 * @brief 通用软件 I2C 驱动 (回调模式)
 *
 * 通过回调函数实现 GPIO 操作, 不依赖特定硬件平台。
 * 用户只需实现 sda_out / sda_in / scl_out 三个回调即可使用。
 */

#ifndef __SW_I2C_H
#define __SW_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** I2C 操作接口 (回调函数表) */
typedef struct {
    void     (*sda_out)(uint8_t bit, void *user_data);
    uint8_t  (*sda_in)(void *user_data);
    void     (*scl_out)(uint8_t bit, void *user_data);
    void     *user_data;
} sw_i2c_interface_t;

/* ---- API ---- */

/** 发送起始条件 */
void i2c_start(sw_i2c_interface_t *i2c);

/** 发送停止条件 */
void i2c_stop(sw_i2c_interface_t *i2c);

/** 发送一个字节, 返回 0=ACK, 1=NACK */
uint8_t i2c_write_byte(sw_i2c_interface_t *i2c, uint8_t dat);

/** 读取一个字节, ack=0 发送 NACK, ack=1 发送 ACK */
uint8_t i2c_read_byte(sw_i2c_interface_t *i2c, uint8_t ack);

/** 向从机写数据 */
uint8_t sw_i2c_write(sw_i2c_interface_t *i2c, uint8_t addr,
                     const uint8_t *data, uint8_t len);

/** 从从机读数据 */
uint8_t sw_i2c_read(sw_i2c_interface_t *i2c, uint8_t addr,
                    uint8_t *data, uint8_t len);

/** 写从机寄存器 (mem_addr=寄存器地址, 然后读 len 字节) */
uint8_t sw_i2c_mem_read(sw_i2c_interface_t *i2c, uint8_t addr,
                        uint8_t mem_addr, uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SW_I2C_H */

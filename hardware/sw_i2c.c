/**
 * @file  sw_i2c.c
 * @brief 通用软件 I2C 驱动实现 (100kHz)
 *
 * 标准 I2C 时序: Start → Addr+R/W → Data → ACK/NACK → Stop
 * 每个 SCL/SDA 操作后 delay_us(5) ≈ 100kHz
 */

#include "sw_i2c.h"
#include "hardware/delay.h"

/* ---- 总线控制 ---- */

#define SDA_OUT(i2c, bit) (i2c)->sda_out((bit), (i2c)->user_data)
#define SDA_IN(i2c)       (i2c)->sda_in((i2c)->user_data)
#define SCL_OUT(i2c, bit) (i2c)->scl_out((bit), (i2c)->user_data)

static void i2c_delay(void)
{
    delay_us(5);  /* 100kHz */
}

void i2c_start(sw_i2c_interface_t *i2c)
{
    SDA_OUT(i2c, 1);
    SCL_OUT(i2c, 1);
    i2c_delay();
    SDA_OUT(i2c, 0);
    i2c_delay();
    SCL_OUT(i2c, 0);
}

void i2c_stop(sw_i2c_interface_t *i2c)
{
    SCL_OUT(i2c, 0);
    SDA_OUT(i2c, 0);
    i2c_delay();
    SCL_OUT(i2c, 1);
    i2c_delay();
    SDA_OUT(i2c, 1);
    i2c_delay();
}

uint8_t i2c_write_byte(sw_i2c_interface_t *i2c, uint8_t dat)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        SDA_OUT(i2c, (dat & 0x80) ? 1 : 0);
        dat <<= 1;
        i2c_delay();
        SCL_OUT(i2c, 1);
        i2c_delay();
        SCL_OUT(i2c, 0);
    }
    /* 读 ACK: SDA=0=ACK, SDA=1=NACK */
    SDA_OUT(i2c, 1);           /* 释放 SDA (开漏, 外部上拉) */
    i2c_delay();
    SCL_OUT(i2c, 1);
    i2c_delay();
    uint8_t ack = SDA_IN(i2c);
    SCL_OUT(i2c, 0);
    return ack;
}

uint8_t i2c_read_byte(sw_i2c_interface_t *i2c, uint8_t ack)
{
    uint8_t i, dat = 0;
    SDA_OUT(i2c, 1);           /* 释放 SDA */
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        SCL_OUT(i2c, 1);
        i2c_delay();
        dat |= SDA_IN(i2c);
        SCL_OUT(i2c, 0);
        i2c_delay();
    }
    /* 发送 ACK/NACK */
    SDA_OUT(i2c, ack ? 0 : 1); /* ack=1→SDA=0(ACK), ack=0→SDA=1(NACK) */
    i2c_delay();
    SCL_OUT(i2c, 1);
    i2c_delay();
    SCL_OUT(i2c, 0);
    return dat;
}

uint8_t sw_i2c_write(sw_i2c_interface_t *i2c, uint8_t addr,
                     const uint8_t *data, uint8_t len)
{
    i2c_start(i2c);
    if (i2c_write_byte(i2c, addr)) {
        i2c_stop(i2c);
        return 1;               /* 从机未应答 */
    }
    for (uint8_t i = 0; i < len; i++) {
        if (i2c_write_byte(i2c, data[i])) {
            i2c_stop(i2c);
            return 1;
        }
    }
    i2c_stop(i2c);
    return 0;
}

uint8_t sw_i2c_read(sw_i2c_interface_t *i2c, uint8_t addr,
                    uint8_t *data, uint8_t len)
{
    i2c_start(i2c);
    if (i2c_write_byte(i2c, addr)) {
        i2c_stop(i2c);
        return 1;
    }
    for (uint8_t i = 0; i < len; i++) {
        data[i] = i2c_read_byte(i2c, i < (len - 1)); /* 最后一字节发 NACK */
    }
    i2c_stop(i2c);
    return 0;
}

uint8_t sw_i2c_mem_read(sw_i2c_interface_t *i2c, uint8_t addr,
                        uint8_t mem_addr, uint8_t *data, uint8_t len)
{
    /* 先写寄存器地址 */
    i2c_start(i2c);
    if (i2c_write_byte(i2c, addr & 0xFE)) { /* 写方向 */
        i2c_stop(i2c);
        return 1;
    }
    if (i2c_write_byte(i2c, mem_addr)) {
        i2c_stop(i2c);
        return 1;
    }
    /* 重复起始 + 读 */
    i2c_start(i2c);
    if (i2c_write_byte(i2c, addr | 0x01)) { /* 读方向 */
        i2c_stop(i2c);
        return 1;
    }
    for (uint8_t i = 0; i < len; i++) {
        data[i] = i2c_read_byte(i2c, i < (len - 1));
    }
    i2c_stop(i2c);
    return 0;
}

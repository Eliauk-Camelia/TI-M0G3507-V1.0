/**
 * @file  gray.c
 * @brief 感为 (GW) 灰度传感器 I2C 驱动实现
 *
 * I2C 地址: 0x4C (7bit), SCL=PA15, SDA=PA16
 *
 * MSPM0 无硬件开漏, 通过动态切换 GPIO 方向模拟:
 *   - 输出 0: 初始化为输出, 清引脚
 *   - 输出 1 (释放): 初始化为输入 (高阻), 外部上拉拉高
 *   - 读取: 初始化为输入, 读引脚电平
 *
 * 关键寄存器:
 *   0xAA  Ping    → 返回 0x66
 *   0xDD  Digital → 返回 1 字节 (bit0~bit7 对应探头 1~8)
 *   0xB0  Analog  → 返回 8 字节 (0~255, 对应探头 1~8)
 */

#include "ti_msp_dl_config.h"
#include "sw_i2c.h"
#include "gray.h"
#include "hardware/delay.h"

/* ---- I2C 传感器地址 ---- */
#define GW_GRAY_ADDR     0x4C
#define GW_GRAY_ADDR_R   ((GW_GRAY_ADDR << 1) | 1)

/* ---- 寄存器 ---- */
#define GW_GRAY_PING           0xAA
#define GW_GRAY_PING_OK        0x66
#define GW_GRAY_DIGITAL_MODE   0xDD
#define GW_GRAY_ANALOG_MODE    0xB0

/* ---- I2C 回调: SCL (PA15) 始终输出 ---- */
static void gray_scl_out(uint8_t bit, void *user_data)
{
    (void)user_data;
    if (bit)
        DL_GPIO_setPins(GRAY_SCL_PORT, GRAY_SCL_PIN_15_PIN);
    else
        DL_GPIO_clearPins(GRAY_SCL_PORT, GRAY_SCL_PIN_15_PIN);
    delay_us(5);
}

/* ---- I2C 回调: SDA (PA16) 开漏模拟 ---- */
static void gray_sda_out(uint8_t bit, void *user_data)
{
    (void)user_data;
    if (bit) {
        /* 释放总线: 切输入 (高阻) → 外部上拉拉高 */
        DL_GPIO_initDigitalInput(GRAY_SDA_PIN_16_IOMUX);
    } else {
        /* 拉低: 切输出 → 清引脚 */
        DL_GPIO_initDigitalOutput(GRAY_SDA_PIN_16_IOMUX);
        DL_GPIO_clearPins(GRAY_SDA_PORT, GRAY_SDA_PIN_16_PIN);
    }
    delay_us(5);
}

/* ---- I2C 回调: SDA 输入 (读 ACK / 数据) ---- */
static uint8_t gray_sda_in(void *user_data)
{
    (void)user_data;
    DL_GPIO_initDigitalInput(GRAY_SDA_PIN_16_IOMUX);
    uint8_t bit = DL_GPIO_readPins(GRAY_SDA_PORT, GRAY_SDA_PIN_16_PIN) ? 1 : 0;
    delay_us(5);
    return bit;
}

/* ---- I2C 接口实例 ---- */
static sw_i2c_interface_t i2c_if = {
    .sda_out   = gray_sda_out,
    .sda_in    = gray_sda_in,
    .scl_out   = gray_scl_out,
    .user_data = NULL,
};

/* ---- API ---- */

int gray_init(void)
{
    uint8_t rsp = 0;

    /* 确保 SCL/SDA 初始为高 (释放) */
    gray_scl_out(1, NULL);
    gray_sda_out(1, NULL);

    /* Ping 直到收到 0x66 */
    while (rsp != GW_GRAY_PING_OK) {
        if (sw_i2c_mem_read(&i2c_if, GW_GRAY_ADDR_R,
                            GW_GRAY_PING, &rsp, 1) == 0) {
            if (rsp == GW_GRAY_PING_OK)
                return 0;
        }
        delay_ms(500);
    }
    return 0;
}

int gray_read_digital(uint8_t *digital)
{
    return sw_i2c_mem_read(&i2c_if, GW_GRAY_ADDR_R,
                           GW_GRAY_DIGITAL_MODE, digital, 1);
}

int gray_read_analog(uint8_t analog[8])
{
    return sw_i2c_mem_read(&i2c_if, GW_GRAY_ADDR_R,
                           GW_GRAY_ANALOG_MODE, analog, 8);
}

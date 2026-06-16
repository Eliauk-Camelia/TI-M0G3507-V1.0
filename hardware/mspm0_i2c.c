/**
 * @file  mspm0_i2c.c
 * @brief MSPM0 硬件 I2C 读写实现 (DL_I2C)
 *
 * SDK v2.10 SysConfig 生成的宏名为 GPIO_I2C_MPU6050_* 前缀。
 */

#include "ti_msp_dl_config.h"
#include "mspm0_i2c.h"
#include "hardware/delay.h"

#define I2C_TIMEOUT_MS  10

/* ---- I2C → GPIO 模式 (用于 SDA 解锁) ---- */

static void i2c_to_gpio(void)
{
    DL_I2C_reset(I2C_MPU6050_INST);
    DL_GPIO_initDigitalOutput(GPIO_I2C_MPU6050_IOMUX_SCL);
    DL_GPIO_initDigitalInputFeatures(GPIO_I2C_MPU6050_IOMUX_SDA,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_clearPins(GPIO_I2C_MPU6050_SCL_PORT, GPIO_I2C_MPU6050_SCL_PIN);
    DL_GPIO_enableOutput(GPIO_I2C_MPU6050_SCL_PORT, GPIO_I2C_MPU6050_SCL_PIN);
}

static void gpio_to_i2c(void)
{
    DL_I2C_reset(I2C_MPU6050_INST);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_MPU6050_IOMUX_SDA,
        GPIO_I2C_MPU6050_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_MPU6050_IOMUX_SCL,
        GPIO_I2C_MPU6050_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_I2C_MPU6050_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_I2C_MPU6050_IOMUX_SCL);
    DL_I2C_enablePower(I2C_MPU6050_INST);
    SYSCFG_DL_I2C_MPU6050_init();
}

/* ---- SDA 解锁 ---- */

void mpu6050_i2c_sda_unlock(void)
{
    uint8_t cnt = 0;
    i2c_to_gpio();
    do {
        DL_GPIO_clearPins(GPIO_I2C_MPU6050_SCL_PORT, GPIO_I2C_MPU6050_SCL_PIN);
        delay_ms(1);
        DL_GPIO_setPins(GPIO_I2C_MPU6050_SCL_PORT, GPIO_I2C_MPU6050_SCL_PIN);
        delay_ms(1);
        if (DL_GPIO_readPins(GPIO_I2C_MPU6050_SDA_PORT, GPIO_I2C_MPU6050_SDA_PIN))
            break;
    } while (++cnt < 100);
    gpio_to_i2c();
}

/* ---- I2C 写 ---- */

int mspm0_i2c_write(uint8_t slave_addr,
                     uint8_t reg_addr,
                     uint8_t length,
                     const uint8_t *data)
{
    if (!length) return 0;

    DL_I2C_transmitControllerData(I2C_MPU6050_INST, reg_addr);
    DL_I2C_clearInterruptStatus(I2C_MPU6050_INST,
        DL_I2C_INTERRUPT_CONTROLLER_TX_DONE);

    while (!(DL_I2C_getControllerStatus(I2C_MPU6050_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE));

    DL_I2C_startControllerTransfer(I2C_MPU6050_INST, slave_addr,
        DL_I2C_CONTROLLER_DIRECTION_TX, length + 1);

    uint8_t cnt = length;
    const uint8_t *ptr = data;
    uint16_t timeout = 0;

    do {
        uint8_t n = DL_I2C_fillControllerTXFIFO(I2C_MPU6050_INST, ptr, cnt);
        cnt -= n;
        ptr += n;
        if (++timeout > (uint16_t)(I2C_TIMEOUT_MS * 100)) {
            mpu6050_i2c_sda_unlock();
            return -1;
        }
        delay_us(10);
    } while (!DL_I2C_getRawInterruptStatus(I2C_MPU6050_INST,
                  DL_I2C_INTERRUPT_CONTROLLER_TX_DONE));

    return 0;
}

/* ---- I2C 读 ---- */

int mspm0_i2c_read(uint8_t slave_addr,
                    uint8_t reg_addr,
                    uint8_t length,
                    uint8_t *data)
{
    if (!length) return 0;

    DL_I2C_transmitControllerData(I2C_MPU6050_INST, reg_addr);
    I2C_MPU6050_INST->MASTER.MCTR = I2C_MCTR_RD_ON_TXEMPTY_ENABLE;
    DL_I2C_clearInterruptStatus(I2C_MPU6050_INST,
        DL_I2C_INTERRUPT_CONTROLLER_RX_DONE);

    while (!(DL_I2C_getControllerStatus(I2C_MPU6050_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE));

    DL_I2C_startControllerTransfer(I2C_MPU6050_INST, slave_addr,
        DL_I2C_CONTROLLER_DIRECTION_RX, length);

    uint8_t i = 0;
    uint16_t timeout = 0;

    do {
        if (!DL_I2C_isControllerRXFIFOEmpty(I2C_MPU6050_INST)) {
            if (i < length) data[i++] = DL_I2C_receiveControllerData(I2C_MPU6050_INST);
        }
        if (++timeout > (uint16_t)(I2C_TIMEOUT_MS * 100)) {
            mpu6050_i2c_sda_unlock();
            return -1;
        }
        delay_us(10);
    } while (!DL_I2C_getRawInterruptStatus(I2C_MPU6050_INST,
                  DL_I2C_INTERRUPT_CONTROLLER_RX_DONE));

    /* 读 FIFO 残留 */
    if (!DL_I2C_isControllerRXFIFOEmpty(I2C_MPU6050_INST)) {
        if (i < length) data[i++] = DL_I2C_receiveControllerData(I2C_MPU6050_INST);
    }

    I2C_MPU6050_INST->MASTER.MCTR = 0;
    DL_I2C_flushControllerTXFIFO(I2C_MPU6050_INST);

    return (i == length) ? 0 : -1;
}

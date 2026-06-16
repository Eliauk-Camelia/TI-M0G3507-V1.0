/**
 * @file  mpu6050.c
 * @brief MPU6050 6 轴传感器驱动实现
 *
 * 纯 I2C 读写, 不使用中断和 DMP。
 * 从加速度计计算 Pitch/Roll, 陀螺仪积分得 Yaw。
 *
 * 参考: ProgramForCar-Main_Develop/Hardware/MPU/
 */

#include "ti_msp_dl_config.h"
#include "mspm0_i2c.h"
#include "mpu6050.h"
#include "hardware/delay.h"
#include <math.h>
#include <string.h>

/* ---- MPU6050 I2C 地址 ---- */
#define MPU6050_ADDR    0x68        /* AD0=GND */

/* ---- 寄存器地址 ---- */
#define MPU6050_RA_WHO_AM_I     0x75
#define MPU6050_RA_PWR_MGMT_1   0x6B
#define MPU6050_RA_SMPLRT_DIV   0x19
#define MPU6050_RA_CONFIG        0x1A
#define MPU6050_RA_GYRO_CONFIG   0x1B
#define MPU6050_RA_ACCEL_CONFIG  0x1C
#define MPU6050_RA_ACCEL_XOUT_H  0x3B
#define MPU6050_RA_GYRO_XOUT_H   0x43

#define MPU6050_ID              0x68

/* ---- 量程和比例因子 ---- */
static float accel_scale = 16384.0f;   /* ±2g: 16384 LSB/g */
static float gyro_scale  = 131.0f;     /* ±250°/s: 131 LSB/(°/s) */

/* ---- 全局变量 ---- */
mpu_angle_t mpu_angle;

/* 上次陀螺仪积分时间戳 (用于 yaw 漂移积分) */
static uint32_t last_tick = 0;

/* ---- 内部函数 ---- */

/** 写单个寄存器 */
static int mpu_write_reg(uint8_t reg, uint8_t val)
{
    return mspm0_i2c_write(MPU6050_ADDR, reg, 1, &val);
}

/** 读连续寄存器 */
static int mpu_read_regs(uint8_t reg, uint8_t len, uint8_t *buf)
{
    return mspm0_i2c_read(MPU6050_ADDR, reg, len, buf);
}

/** 读 WHO_AM_I */
static uint8_t mpu_who_am_i(void)
{
    uint8_t id = 0;
    mpu_read_regs(MPU6050_RA_WHO_AM_I, 1, &id);
    return id;
}

/* ---- 公开 API ---- */

void MPU6050_Init(void)
{
    int retry = 0;

    /* I2C SDA 死锁恢复 */
    mpu6050_i2c_sda_unlock();

    /* 唤醒: 清除 SLEEP 位 */
    while (mpu_write_reg(MPU6050_RA_PWR_MGMT_1, 0x00) != 0) {
        if (++retry > 5) {
            /* I2C 故障, 软复位 MCU */
            DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_POR);
        }
        delay_ms(10);
    }

    delay_ms(100);  /* 等待晶振起振 */

    /* 验证设备 ID */
    if (mpu_who_am_i() != MPU6050_ID) {
        DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_POR);
    }

    /* 采样率分频: 1kHz / (1+4) = 200Hz */
    mpu_write_reg(MPU6050_RA_SMPLRT_DIV, 0x04);

    /* DLPF: 44Hz (Accel) / 42Hz (Gyro) */
    mpu_write_reg(MPU6050_RA_CONFIG, 0x03);

    /* 陀螺仪 ±250°/s */
    mpu_write_reg(MPU6050_RA_GYRO_CONFIG, 0x00);
    gyro_scale = 131.0f;

    /* 加速度计 ±2g */
    mpu_write_reg(MPU6050_RA_ACCEL_CONFIG, 0x00);
    accel_scale = 16384.0f;

    last_tick = 0;
}

void MPU6050_Read(void)
{
    uint8_t buf[14];
    int16_t ax, ay, az, gx, gy, gz;

    /* 读 14 字节: 加速度计(6) + 温度(2) + 陀螺仪(6) */
    if (mpu_read_regs(MPU6050_RA_ACCEL_XOUT_H, 14, buf) != 0)
        return;

    /* 解析 (大端) */
    ax = (int16_t)((buf[0] << 8) | buf[1]);
    ay = (int16_t)((buf[2] << 8) | buf[3]);
    az = (int16_t)((buf[4] << 8) | buf[5]);
    /* 跳过温度 buf[6..7] */
    gx = (int16_t)((buf[8]  << 8) | buf[9]);
    gy = (int16_t)((buf[10] << 8) | buf[11]);
    gz = (int16_t)((buf[12] << 8) | buf[13]);

    /* ---- Pitch / Roll: 由加速度计计算 (静态倾角) ---- */
    float ax_g = ax / accel_scale;
    float ay_g = ay / accel_scale;
    float az_g = az / accel_scale;

    mpu_angle.pitch = atan2f(-ax_g, sqrtf(ay_g * ay_g + az_g * az_g)) * 57.29578f;
    mpu_angle.roll  = atan2f( ay_g, az_g) * 57.29578f;

    /* ---- Yaw: 陀螺仪 Z 轴积分 (累积漂移, 仅做演示) ---- */
    float gz_dps = gz / gyro_scale;  /* °/s */

    if (last_tick == 0) {
        last_tick = 1;  /* 首次不积分 */
    } else {
        /* 简易积分: 假设每次调用间隔已知, 这里用固定 dt */
        mpu_angle.yaw += gz_dps * 0.01f;  /* 10ms 间隔 */
        /* 归一化到 [0, 360) */
        while (mpu_angle.yaw >=  360.0f) mpu_angle.yaw -= 360.0f;
        while (mpu_angle.yaw <    0.0f) mpu_angle.yaw += 360.0f;
    }
}

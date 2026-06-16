/**
 * @file  mpu6050.h
 * @brief MPU6050 6 轴传感器驱动 (I2C, 无中断引脚, 无 DMP)
 *
 * 功能: 初始化 → 读原始数据 → 计算 Pitch/Roll/Yaw
 * I2C 地址: 0x68 (AD0=GND)
 */

#ifndef __MPU6050_H
#define __MPU6050_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 姿态角数据结构 */
typedef struct {
    float pitch;   /* 俯仰角 (度), -90~+90, 由加速度计计算 */
    float roll;    /* 横滚角 (度), -180~+180, 由加速度计计算 */
    float yaw;     /* 偏航角 (度), 由陀螺仪积分, 会漂移 */
} mpu_angle_t;

extern mpu_angle_t mpu_angle;

/**
 * @brief 初始化 MPU6050
 *
 * 步骤: I2C SDA 解锁 → 唤醒 → 配置量程 → 读取验证
 * 失败时无限循环 (软复位)
 */
void MPU6050_Init(void);

/**
 * @brief 读取原始数据并更新姿态角
 *
 * 从 MPU6050 读取 6 轴原始数据 (加速度计 3 轴 + 陀螺仪 3 轴),
 * 计算 Pitch/Roll (加速度计) 和 Yaw (陀螺仪积分)。
 */
void MPU6050_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* __MPU6050_H */

/**
 * @file  mpu6050.h
 * @brief MPU6050 DMP 驱动 (基于 Invensense DMP 库)
 *
 * I2C: PA15=SCL, PA16=SDA, 400kHz
 * 使用 DMP 内部 200Hz 6 轴融合四元数, 无需 MCU 做姿态解算。
 */

#ifndef __MPU6050_H
#define __MPU6050_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float pitch;
    float roll;
    float yaw;
} mpu_angle_t;

extern mpu_angle_t mpu_angle;

/** 初始化 MPU6050 + DMP */
void MPU6050_Init(void);

/** 从 DMP FIFO 读取四元数并计算欧拉角 */
int MPU6050_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* __MPU6050_H */

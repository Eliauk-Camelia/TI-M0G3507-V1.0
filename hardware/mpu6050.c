#include "ti_msp_dl_config.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050.h"
#include "mspm0_i2c.h"
#include "clock.h"
#include "hardware/delay.h"
#include <string.h>
#include <math.h>

#define DEFAULT_MPU_HZ  50
#define q30  1073741824.0f

mpu_angle_t mpu_angle;

static short gyro[3], accel[3];
static long quat[4];
static unsigned long sensor_timestamp;
static short sensors;
static unsigned char more;

static signed char gyro_orientation[9] = {-1, 0, 0, 0,-1, 0, 0, 0, 1};

static inline unsigned short inv_row_2_scale(const signed char *row) {
    unsigned short b;
    if (row[0] > 0) b=0; else if (row[0] < 0) b=4;
    else if (row[1] > 0) b=1; else if (row[1] < 0) b=5;
    else if (row[2] > 0) b=2; else if (row[2] < 0) b=6; else b=7;
    return b;
}
static inline unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx) {
    return inv_row_2_scale(mtx) | (inv_row_2_scale(mtx+3)<<3) | (inv_row_2_scale(mtx+6)<<6);
}
static void tap_cb(unsigned char d, unsigned char c) {}
static void android_orient_cb(unsigned char o) {}

void MPU6050_Init(void)
{
    int result;
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;

    if (DL_I2C_getSDAStatus(I2C_MPU6050_INST) == DL_I2C_CONTROLLER_SDA_LOW)
        mpu6050_i2c_sda_unlock();

    result = mpu_init();
    if (result) DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_POR);

    result = 0;
    result += mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    result += mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    result += mpu_set_sample_rate(DEFAULT_MPU_HZ);
    result += mpu_get_sample_rate(&gyro_rate);
    result += mpu_get_gyro_fsr(&gyro_fsr);
    result += mpu_get_accel_fsr(&accel_fsr);
    result += dmp_load_motion_driver_firmware();
    result += dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));
    result += dmp_register_tap_cb(tap_cb);
    result += dmp_register_android_orient_cb(android_orient_cb);
    result += dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT
                               | DMP_FEATURE_GYRO_CAL
                               | DMP_FEATURE_SEND_CAL_GYRO
                               | DMP_FEATURE_SEND_RAW_ACCEL);
    result += dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    result += mpu_set_dmp_state(1);
    if (result) DL_SYSCTL_resetDevice(DL_SYSCTL_RESET_POR);
}

int MPU6050_Read(void)
{
    int result;
    do {
        result = dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
    } while (more);
    if (result) return -1;

    float q0 = quat[0] / q30, q1 = quat[1] / q30;
    float q2 = quat[2] / q30, q3 = quat[3] / q30;

    mpu_angle.pitch = asinf(-2*q1*q3 + 2*q0*q2) * 57.29578f;
    mpu_angle.roll  = atan2f(2*q2*q3 + 2*q0*q1, -2*q1*q1 - 2*q2*q2 + 1) * 57.29578f;
    mpu_angle.yaw   = atan2f(2*(q1*q2 + q0*q3), q0*q0 + q1*q1 - q2*q2 - q3*q3) * 57.29578f;
    while (mpu_angle.yaw < 0) mpu_angle.yaw += 360;
    return 0;
}

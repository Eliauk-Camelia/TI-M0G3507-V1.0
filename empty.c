/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "hardware/lcd.h"
#include "hardware/lcd_init.h"
#include "hardware/mpu6050.h"

/*
 * MPU6050 姿态角实时显示
 *
 * I2C: PA0=SCL, PA1=SDA, 400kHz
 * 每 100ms 读取一次, 显示 Pitch/Roll/Yaw
 */

int main(void)
{
    SYSCFG_DL_init();
    LCD_Init();

    /* 初始化 MPU6050 */
    MPU6050_Init();

    while (1) {
        /* 读取姿态 */
        MPU6050_Read();

        /* 清屏 */
        LCD_Fill(0, 0, 160, 80, WHITE);

        /* 标题 */
        LCD_ShowString(0, 0, (const uint8_t *)"MPU6050",
                       BLACK, WHITE, 16, 0);

        /* Pitch */
        LCD_ShowString(0, 22, (const uint8_t *)"P:",
                       RED, WHITE, 16, 0);
        LCD_ShowFloatNum1(18, 22, mpu_angle.pitch, 5, BLACK, WHITE, 16);

        /* Roll */
        LCD_ShowString(0, 42, (const uint8_t *)"R:",
                       GREEN, WHITE, 16, 0);
        LCD_ShowFloatNum1(18, 42, mpu_angle.roll, 5, BLACK, WHITE, 16);

        /* Yaw */
        LCD_ShowString(0, 62, (const uint8_t *)"Y:",
                       BLUE, WHITE, 16, 0);
        LCD_ShowFloatNum1(18, 62, mpu_angle.yaw, 5, BLACK, WHITE, 16);

        delay_ms(100);
    }
}

#include "ti_msp_dl_config.h"
#include "hardware/lcd.h"
#include "hardware/lcd_init.h"
#include "hardware/clock.h"
#include "hardware/mpu6050.h"

static void show_angle(uint16_t x, uint16_t y, float val,
                        uint16_t fc, uint16_t bc)
{
    if (val < 0) {
        LCD_ShowString(x, y, (const uint8_t *)"-", fc, bc, 16, 0);
        val = -val; x += 8;
    }
    LCD_ShowFloatNum1(x, y, val, 6, fc, bc, 16);
}

int main(void)
{
    SYSCFG_DL_init();
    LCD_Init();
    MPU6050_Init();

    while (1) {
        if (MPU6050_Read() == 0) {
            LCD_Fill(0, 0, 160, 80, WHITE);
            LCD_ShowString(0, 0, (const uint8_t *)"MPU-DMP", BLACK, WHITE, 16, 0);
            LCD_ShowString(0, 22, (const uint8_t *)"P:", RED, WHITE, 16, 0);
            show_angle(18, 22, mpu_angle.pitch, BLACK, WHITE);
            LCD_ShowString(0, 42, (const uint8_t *)"R:", GREEN, WHITE, 16, 0);
            show_angle(18, 42, mpu_angle.roll, BLACK, WHITE);
            LCD_ShowString(0, 62, (const uint8_t *)"Y:", BLUE, WHITE, 16, 0);
            show_angle(18, 62, mpu_angle.yaw, BLACK, WHITE);
        }
        delay_ms(100);
    }
}

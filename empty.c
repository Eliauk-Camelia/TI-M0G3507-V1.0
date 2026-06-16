#include "ti_msp_dl_config.h"
#include "hardware/lcd.h"
#include "hardware/lcd_init.h"
#include "hardware/clock.h"
#include "hardware/mpu6050.h"

static volatile uint8_t g_refresh = 0;   /* 100ms 刷新标志 */
static volatile uint32_t g_tick = 0;     /* 1ms 滴答 */

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

    /* 启动 1ms 定时器, 用于非阻塞定时 */
    NVIC_ClearPendingIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    DL_TimerG_startCounter(TIMER_0_INST);

    MPU6050_Init();

    while (1) {
        /*
         * 非阻塞模式: 只在 g_refresh 置位时读 MPU6050 和刷新 LCD,
         * 其余时间主循环可做其他事 (读灰度传感器 / 按键扫描 等)
         */
        if (g_refresh) {
            g_refresh = 0;

            if (MPU6050_Read() == 0) {
                LCD_Fill(0, 0, 160, 80, WHITE);
                LCD_ShowString(0, 0, (const uint8_t *)"MPU-DMP",
                               BLACK, WHITE, 16, 0);
                LCD_ShowString(0, 22, (const uint8_t *)"P:",
                               RED, WHITE, 16, 0);
                show_angle(18, 22, mpu_angle.pitch, BLACK, WHITE);
                LCD_ShowString(0, 42, (const uint8_t *)"R:",
                               GREEN, WHITE, 16, 0);
                show_angle(18, 42, mpu_angle.roll, BLACK, WHITE);
                LCD_ShowString(0, 62, (const uint8_t *)"Y:",
                               BLUE, WHITE, 16, 0);
                show_angle(18, 62, mpu_angle.yaw, BLACK, WHITE);
            }
        }

        /* === 此处可添加其他非阻塞任务 === */
        /* 例如: 读灰度传感器、按键扫描、串口通信等 */
    }
}

/* 1ms 中断: 更新 tick_ms + 100ms 刷新标志 */
void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMER_0_INST)) {
        case DL_TIMERG_IIDX_ZERO:
            g_tick++;
            tick_ms = g_tick;  /* 同步 clock.c 的 tick_ms */
            if (g_tick % 100 == 0)
                g_refresh = 1;
            break;
        default:
            break;
    }
}

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

/* ================================================================
 * 全局变量
 * ================================================================ */
volatile uint32_t g_tick_ms = 0;   /* 1ms 系统滴答计数器, 由 TIMER_0 ISR 递增 */
volatile uint8_t  g_lcd_refresh = 0; /* LCD 刷新标志, ISR 置 1, 主循环清 0 */

/* ================================================================
 * 辅助函数声明
 * ================================================================ */
static void demo_text(void);
static void demo_shapes(void);
static void demo_counter(void);

/* ================================================================
 * 主函数
 * ================================================================ */
int main(void)
{
    SYSCFG_DL_init();

    LCD_Init();                              /* 初始化 ST7735S LCD (160x80 横屏) */

    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);   /* 使能 1ms 定时中断 */
    DL_TimerG_startCounter(TIMER_0_INST);    /* 启动 TIMG0 递减计数 */

    /* 开机先展示文字 demo */
    demo_text();

    while (1) {
        /*
         * ★ 关键模式: ISR 只置标志位, 主循环执行耗时操作 ★
         * 将 LCD 通信从 ISR 移到主循环, 避免阻塞其他中断。
         */
        if (g_lcd_refresh) {
            g_lcd_refresh = 0;               /* 清除标志 */

            /* 每 5 秒切换一次 demo 页面 */
            uint32_t phase = (g_tick_ms / 5000) % 3;
            switch (phase) {
                case 0: demo_text();    break;
                case 1: demo_shapes();  break;
                case 2: demo_counter(); break;
                default: break;
            }
        }
    }
}

/* ================================================================
 * TIMER_0 中断服务例程 (1ms)
 * ★ 只做快速操作: 计数 + 置标志位, 不做 LCD 通信 ★
 * ================================================================ */
void TIMER_0_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(TIMER_0_INST)) {
        case DL_TIMERG_IIDX_ZERO:
            g_tick_ms++;
            if (g_tick_ms % 500 == 0) {      /* 每 500ms 触发一次刷新 */
                g_lcd_refresh = 1;
            }
            break;
        default:
            break;
    }
}

/* ================================================================
 * Demo 1: 文字显示 — 展示不同字号和颜色
 * ================================================================
 * LCD_ShowString(x, y, str, fg_color, bg_color, font_size, mode)
 *   font_size: 12(6x12), 16(8x16), 24(12x24), 32(16x32)
 *   mode: 0=带背景填充, 1=透明叠加(不绘制背景)
 */
static void demo_text(void)
{
    /* 清屏为白色背景 */
    LCD_Fill(0, 0, 160, 80, WHITE);

    /* ---- 32 号字: 标题 (带背景色) ---- */
    LCD_ShowString(0, 0,  (const uint8_t *)"LCD Demo",
                   RED, WHITE, 32, 0);

    /* ---- 16 号字: 多行文字 ---- */
    LCD_ShowString(0, 35, (const uint8_t *)"ST7735S",
                   BLUE, WHITE, 16, 0);

    LCD_ShowString(80, 35, (const uint8_t *)"160x80",
                   GREEN, WHITE, 16, 0);

    /* ---- 12 号字: 小字信息 (透明叠加在背景上) ---- */
    LCD_ShowString(0, 55, (const uint8_t *)"MSPM0G3507",
                   GRAY, WHITE, 12, 0);
    LCD_ShowString(0, 68, (const uint8_t *)"Soft-SPI LCD",
                   GRAY, WHITE, 12, 0);
}

/* ================================================================
 * Demo 2: 图形绘制 — 点、线、矩形、圆
 * ================================================================
 * LCD 分辨率: 横屏 160(宽) x 80(高)
 * 坐标原点 (0,0) 在左上角, X 向右, Y 向下
 */
static void demo_shapes(void)
{
    /* 清屏 */
    LCD_Fill(0, 0, 160, 80, WHITE);

    /* 标题 */
    LCD_ShowString(0, 0, (const uint8_t *)"Shapes",
                   BLACK, WHITE, 24, 0);

    /* ---- 线条: 交叉线 ---- */
    LCD_DrawLine(0, 30, 50, 79, RED);       /* 左上 → 左下 */
    LCD_DrawLine(50, 79, 100, 30, GREEN);    /* 左下 → 底部中点偏右 */
    LCD_DrawLine(100, 30, 0, 30, BLUE);      /* 水平线闭合 */

    /* ---- 矩形 ---- */
    LCD_DrawRectangle(110, 35, 155, 75, MAGENTA);  /* 空心矩形 */
    LCD_Fill(120, 45, 145, 65, CYAN);               /* 实心填充矩形 */

    /* ---- 圆 ---- */
    Draw_Circle(40, 55, 18, BROWN);          /* 空心圆 */
    Draw_Circle(80, 55, 10, DARKBLUE);       /* 小圆 */
}

/* ================================================================
 * Demo 3: 动态计数器 — 显示运行时间和数值
 * ================================================================
 * LCD_ShowIntNum(x, y, num, digit_count, fg, bg, font_size)
 *   digit_count: 显示位数, 不足补空格, 超出截断
 */
static void demo_counter(void)
{
    /* 清屏 */
    LCD_Fill(0, 0, 160, 80, WHITE);

    /* 标题 */
    LCD_ShowString(0, 0, (const uint8_t *)"Counter",
                   BLACK, WHITE, 24, 0);

    /* 运行秒数 (浮点数, 1 位小数) */
    float sec = g_tick_ms / 1000.0f;
    LCD_ShowString(0, 30, (const uint8_t *)"Time:",
                   BLUE, WHITE, 16, 0);
    LCD_ShowFloatNum1(55, 30, sec, 5, RED, WHITE, 16);
    LCD_ShowString(140, 30, (const uint8_t *)"s",
                   BLUE, WHITE, 16, 0);

    /* 毫秒计数值 (整数) */
    LCD_ShowString(0, 50, (const uint8_t *)"Ticks:",
                   BLUE, WHITE, 16, 0);
    LCD_ShowIntNum(60, 50, (uint16_t)(g_tick_ms % 100000), 5,
                   RED, WHITE, 16);
}

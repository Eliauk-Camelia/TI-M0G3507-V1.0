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
#include "hardware/grey.h"

/*
 * 感为灰度传感器 串行模式
 *
 * CLK=PA24(AD0)  DAT=PA25(AD1)
 * 每 200ms 读取一次 8 路数字量 (0=黑, 1=白)
 */

int main(void)
{
    SYSCFG_DL_init();
    LCD_Init();

    /* 初始化传感器串行模式 */
    grey_init();
     /* 读取 8 路数字量 */
    uint8_t digital = grey_read_digital();
    /* 清屏 */
    LCD_Fill(0, 0, 160, 80, WHITE);

    /* 标题 */
    LCD_ShowString(0, 0, (const uint8_t *)"GW Serial",
                    BLACK, WHITE, 16, 0);

    while (1) {
       
        
        /* 8 通道: 4列 × 2行 */
        for (int i = 0; i < 8; i++) {
            int col = i % 4;
            int row = i / 4;
            uint16_t x = 2 + col * 40;
            uint16_t y = 22 + row * 28;

            LCD_ShowIntNum(x, y, i + 1, 1, BLACK, WHITE, 16);
            LCD_ShowString(x + 8, y, (const uint8_t *)":",
                           GRAY, WHITE, 16, 0);
            /* 显示 0 或 1 */
            LCD_ShowIntNum(x + 16, y, (digital >> i) & 1, 1,
                           RED, WHITE, 16);
        }

        delay_ms(200);
    }
}

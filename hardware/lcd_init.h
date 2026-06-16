/**
 * @file  lcd_init.h
 * @brief ST7735S LCD 初始化模块 — 软件 SPI (GPIO 模拟)
 *
 * 引脚映射 (用户电路):
 *   PA23 → RES   PA28 → DC    PA31 → CS
 *   PB19 → BLK   PB20 → SCK   PB12 → MOSI
 *
 * 背光: PNP 三极管 (SS8550), 低电平点亮, 高电平关闭
 */


#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include "ti_msp_dl_config.h"
#include "hardware/delay.h"
#include "hardware/lcd.h"

/* ---- 屏幕方向: 0/1=竖屏, 2/3=横屏 ---- */
#define USE_HORIZONTAL  2

#if USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1
#define LCD_W  80
#define LCD_H  160
#else
#define LCD_W  160
#define LCD_H  80
#endif

/* ---- 软件 SPI 引脚宏 — 使用 SysConfig 生成的 GPIO 宏 ---- */
/* LCD_A = Port A: RST(PA23) / DC(PA28) / CS(PA31)                 */
/* LCD_B = Port B: SCK(PB20) / MOSI(PB12) / BLK(PB19)              */

#define LCD_SCK_Clr()  DL_GPIO_clearPins(LCD_B_PORT, LCD_B_SCK_PIN)
#define LCD_SCK_Set()  DL_GPIO_setPins(LCD_B_PORT, LCD_B_SCK_PIN)

#define LCD_MOSI_Clr() DL_GPIO_clearPins(LCD_B_PORT, LCD_B_MOSI_PIN)
#define LCD_MOSI_Set() DL_GPIO_setPins(LCD_B_PORT, LCD_B_MOSI_PIN)

#define LCD_RES_Clr()  DL_GPIO_clearPins(LCD_A_PORT, LCD_A_RST_PIN)
#define LCD_RES_Set()  DL_GPIO_setPins(LCD_A_PORT, LCD_A_RST_PIN)

#define LCD_DC_Clr()   DL_GPIO_clearPins(LCD_A_PORT, LCD_A_DC_PIN)
#define LCD_DC_Set()   DL_GPIO_setPins(LCD_A_PORT, LCD_A_DC_PIN)

#define LCD_CS_Clr()   DL_GPIO_clearPins(LCD_A_PORT, LCD_A_CS_PIN)
#define LCD_CS_Set()   DL_GPIO_setPins(LCD_A_PORT, LCD_A_CS_PIN)

#define LCD_BLK_Clr()  DL_GPIO_clearPins(LCD_B_PORT, LCD_B_BLK_PIN)
#define LCD_BLK_Set()  DL_GPIO_setPins(LCD_B_PORT, LCD_B_BLK_PIN)

/* ---- API ---- */
void LCD_Writ_Bus(uint8_t dat);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA(uint16_t dat);
void LCD_WR_REG(uint8_t dat);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_Init(void);

#endif /* __LCD_INIT_H */

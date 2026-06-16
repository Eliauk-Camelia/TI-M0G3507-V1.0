/**
 * @file  lcd_init.h
 * @brief ST7735S LCD 初始化模块 — 软件 SPI (GPIO 模拟)
 *
 * 引脚映射:
 *   PB19 → BLK (背光, PNP 管, 低电平点亮)
 *   PA23 → CS  (片选)
 *   PB24 → DC  (数据/命令)
 *   PB20 → RES (复位)
 *   PA28 → MOSI (数据)
 *   PA31 → SCLK (时钟)
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

#define LCD_SCK_Clr()  DL_GPIO_clearPins(SCLK_PORT, SCLK_PIN_31_PIN)
#define LCD_SCK_Set()  DL_GPIO_setPins(SCLK_PORT, SCLK_PIN_31_PIN)

#define LCD_MOSI_Clr() DL_GPIO_clearPins(MOSI_PORT, MOSI_PIN_28_PIN)
#define LCD_MOSI_Set() DL_GPIO_setPins(MOSI_PORT, MOSI_PIN_28_PIN)

#define LCD_RES_Clr()  DL_GPIO_clearPins(RES_PORT, RES_PIN_20_PIN)
#define LCD_RES_Set()  DL_GPIO_setPins(RES_PORT, RES_PIN_20_PIN)

#define LCD_DC_Clr()   DL_GPIO_clearPins(DC_PORT, DC_PIN_24_PIN)
#define LCD_DC_Set()   DL_GPIO_setPins(DC_PORT, DC_PIN_24_PIN)

#define LCD_CS_Clr()   DL_GPIO_clearPins(CS_PORT, CS_PIN_23_PIN)
#define LCD_CS_Set()   DL_GPIO_setPins(CS_PORT, CS_PIN_23_PIN)

#define LCD_BLK_Clr()  DL_GPIO_clearPins(BLK_PORT, BLK_PIN_19_PIN)
#define LCD_BLK_Set()  DL_GPIO_setPins(BLK_PORT, BLK_PIN_19_PIN)

/* ---- API ---- */
void LCD_Writ_Bus(uint8_t dat);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA(uint16_t dat);
void LCD_WR_REG(uint8_t dat);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_Init(void);

#endif /* __LCD_INIT_H */

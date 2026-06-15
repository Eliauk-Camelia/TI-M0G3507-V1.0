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

/* ---- 软件 SPI 引脚宏 (GPIO 位操作) ---- */
#define LCD_SCK_Clr()  DL_GPIO_clearPins(LCDSCK_PORT, LCDSCK_B20_PIN)
#define LCD_SCK_Set()  DL_GPIO_setPins(LCDSCK_PORT, LCDSCK_B20_PIN)

#define LCD_MOSI_Clr() DL_GPIO_clearPins(LCDMOSI_PORT, LCDMOSI_B12_PIN)
#define LCD_MOSI_Set() DL_GPIO_setPins(LCDMOSI_PORT, LCDMOSI_B12_PIN)

#define LCD_RES_Clr()  DL_GPIO_clearPins(LCDRST_PORT, LCDRST_A23_PIN)
#define LCD_RES_Set()  DL_GPIO_setPins(LCDRST_PORT, LCDRST_A23_PIN)

#define LCD_DC_Clr()   DL_GPIO_clearPins(LCDDC_PORT, LCDDC_A28_PIN)
#define LCD_DC_Set()   DL_GPIO_setPins(LCDDC_PORT, LCDDC_A28_PIN)

#define LCD_CS_Clr()   DL_GPIO_clearPins(LCDCS_PORT, LCDCS_A31_PIN)
#define LCD_CS_Set()   DL_GPIO_setPins(LCDCS_PORT, LCDCS_A31_PIN)

#define LCD_BLK_Clr()  DL_GPIO_clearPins(LCDBLK_PORT, LCDBLK_B19_PIN)
#define LCD_BLK_Set()  DL_GPIO_setPins(LCDBLK_PORT, LCDBLK_B19_PIN)

/* ---- API ---- */
void LCD_Writ_Bus(uint8_t dat);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA(uint16_t dat);
void LCD_WR_REG(uint8_t dat);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_Init(void);

#endif /* __LCD_INIT_H */

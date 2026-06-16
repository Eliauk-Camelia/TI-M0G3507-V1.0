/**
 * @file  lcd.h
 * @brief ST7735S LCD 绘图函数 + 颜色定义
 */

#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>


/* ---- 颜色 (RGB565) ---- */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define MAGENTA     0xF81F
#define GRAY        0x8430
#define BRED        0xF81F
#define GRED        0xFFE0
#define GBLUE       0x07FF
#define BROWN       0xBC40
#define BRRED       0xFC07
#define DARKBLUE    0x01CF
#define LIGHTBLUE   0x7D7C
#define GRAYBLUE    0x5458
#define LIGHTGREEN  0x841F
#define LGRAY       0xC618
#define LGRAYBLUE   0xA651
#define LBBLUE      0x2B12

/* ---- 绘图 API ---- */
void LCD_Fill(uint16_t xsta, uint16_t ysta,
              uint16_t xend, uint16_t yend, uint16_t color);

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);

void LCD_DrawLine(uint16_t x1, uint16_t y1,
                  uint16_t x2, uint16_t y2, uint16_t color);

void LCD_DrawRectangle(uint16_t x1, uint16_t y1,
                       uint16_t x2, uint16_t y2, uint16_t color);

void Draw_Circle(uint16_t x0, uint16_t y0,
                 uint8_t r, uint16_t color);

/* ---- 文字显示 ---- */
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num,
                  uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);

void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p,
                    uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);

uint32_t mypow(uint8_t m, uint8_t n);

void LCD_ShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                    uint16_t fc, uint16_t bc, uint8_t sizey);

void LCD_ShowFloatNum1(uint16_t x, uint16_t y, float num, uint8_t len,
                       uint16_t fc, uint16_t bc, uint8_t sizey);

/* ---- 图片显示 ---- */
void LCD_ShowPicture(uint16_t x, uint16_t y,
                     uint16_t length, uint16_t width, const uint8_t pic[]);

#endif /* __LCD_H */

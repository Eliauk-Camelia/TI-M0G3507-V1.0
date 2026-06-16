/**
 * @file  lcd.c
 * @brief ST7735S LCD 绘图函数实现
 *
 * 基于官方例程 BSP/lcd.c, 适配 MSPM0G3507 DeviceLib。
 * 所有写屏操作通过 lcd_init.c 的软件 SPI 完成。
 */

#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"


/* ===================================================================
 * 区域填充
 * =================================================================== */

void LCD_Fill(uint16_t xsta, uint16_t ysta,
              uint16_t xend, uint16_t yend, uint16_t color)
{
    uint16_t i, j;
    LCD_Address_Set(xsta, ysta, xend - 1, yend - 1);
    for (i = ysta; i < yend; i++) {
        for (j = xsta; j < xend; j++) {
            LCD_WR_DATA(color);
        }
    }
}

/* ===================================================================
 * 画点
 * =================================================================== */

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA(color);
}

/* ===================================================================
 * 画线 (Bresenham)
 * =================================================================== */

void LCD_DrawLine(uint16_t x1, uint16_t y1,
                  uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow    = x1;
    uCol    = y1;

    if (delta_x > 0)      incx = 1;
    else if (delta_x == 0) incx = 0;
    else { incx = -1; delta_x = -delta_x; }

    if (delta_y > 0)      incy = 1;
    else if (delta_y == 0) incy = 0;
    else { incy = -1; delta_y = -delta_y; }

    distance = (delta_x > delta_y) ? delta_x : delta_y;

    for (t = 0; t <= distance; t++) {
        LCD_DrawPoint(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance) { xerr -= distance; uRow += incx; }
        if (yerr > distance) { yerr -= distance; uCol += incy; }
    }
}

/* ===================================================================
 * 画矩形
 * =================================================================== */

void LCD_DrawRectangle(uint16_t x1, uint16_t y1,
                       uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

/* ===================================================================
 * 画圆 (Bresenham)
 * =================================================================== */

void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a, b, di;
    a = 0;
    b = r;
    di = 3 - (r << 1);
    while (a <= b) {
        LCD_DrawPoint(x0 + a, y0 - b, color);
        LCD_DrawPoint(x0 + b, y0 - a, color);
        LCD_DrawPoint(x0 + b, y0 + a, color);
        LCD_DrawPoint(x0 + a, y0 + b, color);
        LCD_DrawPoint(x0 - a, y0 + b, color);
        LCD_DrawPoint(x0 - b, y0 + a, color);
        LCD_DrawPoint(x0 - a, y0 - b, color);
        LCD_DrawPoint(x0 - b, y0 - a, color);
        a++;
        if (di < 0) {
            di += 4 * a + 6;
        } else {
            di += 10 + 4 * (a - b);
            b--;
        }
    }
}

/* ===================================================================
 * 显示单个 ASCII 字符
 * =================================================================== */

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num,
                  uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t temp, sizex, t;
    uint16_t i, TypefaceNum;
    uint16_t x0 = x;

    /* 仅允许可打印 ASCII 字符, 防止数组越界 */
    if (num < ' ' || num > '~') return;

    sizex = sizey / 2;
    TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
    num = num - ' ';  /* 偏移到字库起始位置 */

    LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);
    for (i = 0; i < TypefaceNum; i++) {
        if (sizey == 12) temp = ascii_1206[num][i];
        else if (sizey == 16) temp = ascii_1608[num][i];
        else if (sizey == 24) temp = ascii_2412[num][i];
        else if (sizey == 32) temp = ascii_3216[num][i];
        else return;

        for (t = 0; t < 8; t++) {
            if (!mode) {
                LCD_WR_DATA((temp & 0x80) ? fc : bc);
                temp <<= 1;
            } else {
                if (temp & 0x80)
                    LCD_DrawPoint(x, y, fc);
                temp <<= 1;
                x++;
                if ((x - x0) == sizex) {
                    x = x0;
                    y++;
                    break;
                }
            }
        }
    }
}

/* ===================================================================
 * 显示字符串
 * =================================================================== */

void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p,
                    uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    while (*p != '\0') {
        LCD_ShowChar(x, y, *p, fc, bc, sizey, mode);
        x += sizey / 2;
        p++;
    }
}

/* ===================================================================
 * 辅助: 幂运算
 * =================================================================== */

uint32_t mypow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--) result *= m;
    return result;
}

/* ===================================================================
 * 显示整数
 * =================================================================== */

void LCD_ShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                    uint16_t fc, uint16_t bc, uint8_t sizey)
{
    uint8_t t, enshow = 0;
    for (t = 0; t < len; t++) {
        uint8_t digit = (num / mypow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1)) {
            if (digit == 0) {
                LCD_ShowChar(x + (sizey / 2) * t, y, ' ', fc, bc, sizey, 0);
                continue;
            }
            enshow = 1;
        }
        LCD_ShowChar(x + (sizey / 2) * t, y, digit + '0', fc, bc, sizey, 0);
    }
}

/* ===================================================================
 * 显示浮点数 (1 位小数)
 * =================================================================== */

void LCD_ShowFloatNum1(uint16_t x, uint16_t y, float num, uint8_t len,
                       uint16_t fc, uint16_t bc, uint8_t sizey)
{
    uint16_t intPart  = (uint16_t)num;
    uint16_t decPart  = (uint16_t)((num - intPart) * 10);
    LCD_ShowIntNum(x, y, intPart, len, fc, bc, sizey);
    LCD_ShowChar(x + (sizey / 2) * len, y, '.', fc, bc, sizey, 0);
    LCD_ShowIntNum(x + (sizey / 2) * (len + 1), y, decPart, 1, fc, bc, sizey);
}

/* ===================================================================
 * 显示图片 (位图)
 * =================================================================== */

void LCD_ShowPicture(uint16_t x, uint16_t y,
                     uint16_t length, uint16_t width, const uint8_t pic[])
{
    uint16_t i, j;
    uint32_t k = 0;
    LCD_Address_Set(x, y, x + length - 1, y + width - 1);
    for (i = 0; i < length; i++) {
        for (j = 0; j < width; j++) {
            LCD_WR_DATA(pic[k * 2] << 8 | pic[k * 2 + 1]);
            k++;
        }
    }
}

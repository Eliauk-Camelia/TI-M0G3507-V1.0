/**
 * @file  lcd_init.c
 * @brief ST7735S 软件 SPI 驱动 + 初始化序列
 *
 * 使用 GPIO 模拟 SPI 时序 (PB20=SCK, PB24=MOSI, PA31=CS)。
 * 官方例程已验证此方式在 MSPM0G3507 上可靠工作。
 */

#include "lcd_init.h"

/* ===================================================================
 * 软件 SPI — 发送一个字节
 * =================================================================== */

void LCD_Writ_Bus(uint8_t dat)
{
    uint8_t i;
    LCD_CS_Clr();
    for (i = 0; i < 8; i++) {
        LCD_SCK_Clr();
        if (dat & 0x80) {
            LCD_MOSI_Set();
        } else {
            LCD_MOSI_Clr();
        }
        LCD_SCK_Set();
        dat <<= 1;
    }
    LCD_CS_Set();
}

/* ---- 写数据 (8位 / 16位) ---- */

void LCD_WR_DATA8(uint8_t dat)  { LCD_Writ_Bus(dat); }

void LCD_WR_DATA(uint16_t dat)
{
    LCD_Writ_Bus(dat >> 8);
    LCD_Writ_Bus(dat);
}

/* ---- 写命令 (DC=0 时发送) ---- */

void LCD_WR_REG(uint8_t dat)
{
    LCD_DC_Clr();
    LCD_Writ_Bus(dat);
    LCD_DC_Set();
}

/* ---- 设置显示窗口 ---- */

void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WR_REG(0x2A);           /* 列地址 */
    LCD_WR_DATA(x1 + 1);
    LCD_WR_DATA(x2 + 1);
    LCD_WR_REG(0x2B);           /* 行地址 */
    LCD_WR_DATA(y1 + 26);
    LCD_WR_DATA(y2 + 26);
    LCD_WR_REG(0x2C);           /* 内存写 */
}

/* ===================================================================
 * ST7735S 初始化序列
 * =================================================================== */

void LCD_Init(void)
{
    /* 硬件复位 */
    LCD_RES_Clr();
    delay_ms(100);
    LCD_RES_Set();
    delay_ms(100);

    /* 打开背光 (BLK=0 → PNP 管导通) */
    LCD_BLK_Clr();
    delay_ms(100);

    /* ---- ST7735S 寄存器配置 ---- */
    LCD_WR_REG(0x11);           /* Sleep Out */
    delay_ms(120);
    LCD_WR_REG(0xB1);           /* Frame Rate: Normal mode */
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    LCD_WR_REG(0xB2);           /* Frame Rate: Idle mode */
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    LCD_WR_REG(0xB3);           /* Frame Rate: Partial mode */
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    LCD_WR_REG(0xB4);           /* Dot Inversion */
    LCD_WR_DATA8(0x03);
    LCD_WR_REG(0xC0);           /* Power: AVDD GVDD */
    LCD_WR_DATA8(0xAB);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x04);
    LCD_WR_REG(0xC1);           /* Power: VGH VGL */
    LCD_WR_DATA8(0xC5);
    LCD_WR_REG(0xC2);           /* Power: Normal Mode */
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x00);
    LCD_WR_REG(0xC3);           /* Power: Idle */
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0x6A);
    LCD_WR_REG(0xC4);           /* Power: Partial+Full */
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0xEE);
    LCD_WR_REG(0xC5);           /* VCOM */
    LCD_WR_DATA8(0x0F);
    LCD_WR_REG(0xE0);           /* Gamma (+) */
    LCD_WR_DATA8(0x07); LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x08); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x10); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x02); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x09); LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x25); LCD_WR_DATA8(0x36);
    LCD_WR_DATA8(0x00); LCD_WR_DATA8(0x08);
    LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x10);
    LCD_WR_REG(0xE1);           /* Gamma (-) */
    LCD_WR_DATA8(0x0A); LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x08); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x0F); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x02); LCD_WR_DATA8(0x07);
    LCD_WR_DATA8(0x09); LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x25); LCD_WR_DATA8(0x35);
    LCD_WR_DATA8(0x00); LCD_WR_DATA8(0x09);
    LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x10);
    LCD_WR_REG(0xFC);
    LCD_WR_DATA8(0x80);
    LCD_WR_REG(0x3A);           /* Pixel Format: 16-bit */
    LCD_WR_DATA8(0x05);
    LCD_WR_REG(0x36);           /* MADCTL: 方向 */
    LCD_WR_DATA8(0x78);         /* 横屏 */
    LCD_WR_REG(0x21);           /* Display Inversion */
    LCD_WR_REG(0x29);           /* Display On */

    /* 清屏为白色 */
    LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);
}

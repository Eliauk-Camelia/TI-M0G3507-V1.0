# TI-M0G3507-V1.0

基于 TI MSPM0G3507 的嵌入式固件工程，集成感为灰度传感器（串行模式）+ ST7735S LCD 实时显示。

## 硬件平台

| 项目 | 说明 |
|------|------|
| **MCU** | MSPM0G3507 (Cortex-M0+, 32 MHz) |
| **开发板** | 天孟星 (LCKFB) MSPM0G3507 核心板 + 机器人扩展板 |
| **调试器** | XDS110 (SWD: PA20=SWCLK, PA19=SWDIO) |
| **LCD** | ST7735S (160×80, 横屏, RGB565) |
| **灰度传感器** | 感为 8 路灰度 + 辅助板 (串行 CLK+DAT 协议) |

## 工具链

| 工具 | 版本 |
|------|------|
| **IDE** | CCS Theia 70.4.0 |
| **编译器** | TI Arm Clang 4.0.4.LTS |
| **SysConfig** | 1.26.2 |
| **MSPM0 SDK** | 2.10.00.04 |
| **构建系统** | GNU Make (Debug 配置, -O2 优化) |

## 时钟配置

```
SYSOSC (32 MHz) → SYSPLL ×2 → ÷2 → ÷2 → 32 MHz CPUCLK
```

| 参数 | 值 |
|------|-----|
| CPUCLK | 32 MHz |
| ULPCLK | 32 kHz |

## 引脚映射

### 调试接口

| 功能 | 引脚 | 说明 |
|------|------|------|
| SWCLK | PA20 | 调试时钟 |
| SWDIO | PA19 | 调试数据 |

### 灰度传感器 (感为辅助板 串行模式)

| 功能 | 引脚 | 实例名 | 方向 | 说明 |
|------|------|--------|------|------|
| CLK | PA24 | Grey (AD0) | 输出 | 时钟信号, MCU 产生 |
| DAT | PA25 | DAT (PIN_25) | 输入 NONE | 数据信号, 辅助板驱动, 悬浮输入 |

- 2 线串行协议：CLK 下降沿读 DAT, 上升沿辅助板更新下一位
- **首时钟丢弃**：读取前先发一个 dummy 时钟周期, 否则数据右移一位
- 返回 8 位数字量 (bit0~bit7 对应探头 1~8, 0=黑, 1=白)
- 参考例程：`/home/arch/work-space/stm32-soace/感为灰度STM32F1xx标准库例程/`
- 辅助板教程：https://www.bilibili.com/video/BV1ZdHSzCEGN/

### LCD (ST7735S 软件 SPI)

| 功能 | 引脚 | 实例名 | 说明 |
|------|------|--------|------|
| SCLK | PA31 | SCLK | 软件 SPI 时钟 |
| MOSI | PA28 | MOSI | 软件 SPI 数据 |
| CS   | PA23 | CS | 片选 (低有效) |
| DC   | PB24 | DC | 数据/命令选择 |
| RES  | PB20 | RES | 硬件复位 (低有效) |
| BLK  | PB19 | BLK | 背光 PNP 管 (低电平点亮) |

**关键注意：**
- LCD 引脚使用**独立 GPIO 实例**，不可用分组 (`LCD_A`/`LCD_B`)
- `lcd_init.h` 中的宏名必须与 SysConfig 实例名一致
- MADCTL 固定为 `0x78` (横屏方向)
- `LCD_ShowChar` 使用 LSB 优先 (`temp & 0x01`) 逐点绘制，避免字符镜像和 12px 字体错位
- Mode=0 必须同时绘制背景像素，否则旧数据残留

## 项目结构

```
TI-M0G3507-V1.0/
├── empty.c              # 主程序 — 灰度传感器串行读取 + LCD 显示
├── empty.syscfg         # SysConfig 外设配置
├── hardware/
│   ├── delay.h/c        # delay_ms/delay_us (delay_cycles 忙等待)
│   ├── grey.h/c         # 灰度传感器串行接口驱动 (CLK+DAT)
│   ├── gray.h/c         # 灰度传感器 I2C 驱动 (备用, 当前未使用)
│   ├── sw_i2c.h/c       # 通用软件 I2C 驱动 (备用, 当前未使用)
│   ├── lcd.h/c          # ST7735S 绘图函数 (Fill/Line/Circle/String/Int/Float)
│   ├── lcd_init.h/c     # ST7735S 软件 SPI + 初始化序列
│   ├── lcdfont.h        # ASCII 字库 (12×6 / 16×8 / 24×12 / 32×16)
│   └── delay.h/c        # 毫秒/微秒忙等待延时
├── Debug/               # 构建产物 (makefile 等, 自动生成)
└── targetConfigs/       # XDS110 调试配置
```

## SysConfig 外设一览

| 模块 | 实例名 | 用途 |
|------|--------|------|
| SYSCTL | — | 时钟树 (32 MHz) |
| GPIO | Grey | CLK 时钟输出 (PA24) |
| GPIO | DAT | DAT 数据输入, 悬浮 (PA25) |
| GPIO | BLK | LCD 背光 (PB19) |
| GPIO | CS | LCD 片选 (PA23) |
| GPIO | DC | LCD 数据/命令 (PB24) |
| GPIO | RES | LCD 复位 (PB20) |
| GPIO | MOSI | LCD 数据 (PA28) |
| GPIO | SCLK | LCD 时钟 (PA31) |
| Board | — | 调试接口 (PA19/PA20) |

## 构建与烧录

```bash
# 构建
cd Debug && gmake -j$(nproc) all

# 清理
gmake -C Debug clean

# 烧录 (XDS110)
dslite -c targetConfigs/MSPM0G3507.ccxml -e -r 2 -u Debug/TI-M0G3507-V1.0.out
```

## 程序运行流程

```
SYSCFG_DL_init()           ← SysConfig 生成的时钟+GPIO 初始化

main()
    ├── LCD_Init()         ← ST7735S 初始化 + 清屏
    ├── grey_init()        ← 灰度传感器串行模式唤醒
    ├── LCD_Fill()         ← 清屏
    └── while(1)
        ├── grey_read_digital()   ← 读 8 路探头数字量 (0/1)
        └── LCD 显示 4×2 网格数据
            delay_ms(200)         ← 200ms 刷新间隔
```

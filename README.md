# TI-M0G3507-V1.0

基于 TI MSPM0G3507 的嵌入式固件工程，集成灰度传感器采集、ST7735S LCD 显示与 1ms 系统滴答定时器。

## 硬件平台

| 项目 | 说明 |
|------|------|
| **MCU** | MSPM0G3507 (Cortex-M0+, 80 MHz max) |
| **开发板** | LP_MSPM0G3507 LaunchPad |
| **调试器** | XDS110 (SWD) |
| **LCD** | ST7735S (160×80, 横屏, RGB565) |
| **ADC 外设** | 8 通道模拟开关 (CD4051) + 灰度传感器 |

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

### 灰度传感器 (ADC + 模拟开关)

| 功能 | 引脚 | 说明 |
|------|------|------|
| ADC_IN | PA27 | ADC0 模拟输入 |
| AD0 | PA24 | 模拟开关地址线 bit0 |
| AD1 | PA25 | 模拟开关地址线 bit1 |
| AD2 | PA26 | 模拟开关地址线 bit2 |

- 8 路传感器通过 CD4051 切换，`grey.c` 中 `read_all_adc()` 一次读取全部 8 通道
- ADC 模式：单次软件触发、轮询等待、12 位精度

### LCD (ST7735S 软件 SPI)

| 功能 | 引脚 | 端口 |
|------|------|------|
| SCK  | PB20 | Port B |
| MOSI | PB12 | Port B |
| RST  | PA23 | Port A |
| DC   | PA28 | Port A |
| CS   | PA31 | Port A |
| BLK  | PB19 | Port B (低电平点亮) |

- 软件模拟 SPI 时序，使用 DriverLib GPIO 位操作
- 横屏模式 (USE_HORIZONTAL=2)，分辨率 160×80

### 系统定时器

| 参数 | 值 |
|------|-----|
| 外设 | TIMG0 |
| 周期 | 1 ms |
| 时钟源 | BUSCLK / 8 (4 MHz) |
| 中断 | ZERO 事件 |

## 项目结构

```
TI-M0G3507-V1.0/
├── empty.c              # 主程序入口 (main, ISR)
├── empty.syscfg         # SysConfig 外设配置文件
├── hardware/
│   ├── grey.c / grey.h         # 灰度传感器 ADC 读取
│   ├── lcd.c / lcd.h           # ST7735S 绘图函数 (点/线/圆/文字/图片)
│   ├── lcd_init.c / lcd_init.h # ST7735S 软件 SPI 驱动 + 初始化序列
│   ├── lcdfont.h               # ASCII 字库 (12×6 / 16×8 / 24×12 / 32×16)
│   └── delay.c / delay.h       # 毫秒/微秒忙等待延时
├── Debug/               # 构建输出 (自动生成，勿手动修改)
├── targetConfigs/       # CCS 调试配置 (XDS110)
└── README.md
```

## SysConfig 外设一览

| 模块 | 实例名 | 用途 |
|------|--------|------|
| SYSCTL | — | 时钟树 (32 MHz) |
| ADC12 | Grey_ADC | 灰度传感器采样 (PA27) |
| GPIO | Grey | 模拟开关地址线 (PA24/PA25/PA26) |
| GPIO | LCD_A | LCD 控制线 Port A (RST/DC/CS) |
| GPIO | LCD_B | LCD 控制线 Port B (SCK/MOSI/BLK) |
| TIMER | TIMER_0 | 1ms 系统滴答 (TIMG0) |
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
SYSCFG_DL_init()        ← SysConfig 生成的外设初始化
    ├── 时钟 (32 MHz)
    ├── GPIO (Grey/LCD_A/LCD_B 引脚输出)
    ├── ADC12 (Grey_ADC 配置)
    └── TIMER_0 (1ms 周期中断)

main()
    ├── LCD_Init()       ← ST7735S 初始化序列 + 清屏
    ├── NVIC_EnableIRQ() ← 使能 1ms 定时中断
    ├── DL_TimerG_startCounter()
    └── while(1) {}      ← 主循环

TIMER_0_INST_IRQHandler  ← 每 1ms 触发
    └── g_tick_ms++
```

# TI-M0G3507-V1.0

基于 TI MSPM0G3507 的嵌入式固件工程，集成 MPU6050 姿态传感器 + 感为灰度传感器 + ST7735S LCD 显示。

## 硬件平台

| 项目 | 说明 |
|------|------|
| **MCU** | MSPM0G3507 (Cortex-M0+, 32 MHz) |
| **开发板** | 天孟星 (LCKFB) MSPM0G3507 核心板 + 机器人扩展板 |
| **调试器** | XDS110 (SWD: PA20=SWCLK, PA19=SWDIO) |
| **LCD** | ST7735S (160×80, 横屏, RGB565) |
| **灰度传感器** | 感为 8 路灰度 + 辅助板 (串行 CLK+DAT 协议) |
| **姿态传感器** | MPU6050 6 轴 (I2C, 400kHz, 无 DMP/中断) |

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
- 参考：`/home/arch/work-space/stm32-soace/感为灰度STM32F1xx标准库例程/`

### MPU6050 (硬件 I2C)

| 功能 | 引脚 | 实例名 | 说明 |
|------|------|--------|------|
| SCL | PA15 | I2C_MPU6050 | I2C 时钟, 400kHz Fast Mode |
| SDA | PA16 | I2C_MPU6050 | I2C 数据 |

- I2C 地址: 0x68 (AD0=GND)
- 不使用中断引脚, 轮询读取
- 不使用 DMP (数字运动处理器), 直接读原始数据计算姿态
- Pitch/Roll 由加速度计计算, Yaw 由陀螺仪 Z 轴积分
- 读取频率: 10Hz (每 100ms)
- 参考：`/home/arch/work-space/project-space/ProgramForCar-Main_Develop/`

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
- LCD 引脚使用**独立 GPIO 实例**，不可用分组
- `lcd_init.h` 中的宏名必须与 SysConfig 实例名一致
- MADCTL 固定为 `0x78` (横屏方向)
- `LCD_ShowChar` 使用 LSB 优先 (`temp & 0x01`) 逐点绘制
- Mode=0 必须同时绘制背景像素，否则旧数据残留

## 项目结构

```
TI-M0G3507-V1.0/
├── empty.c              # 主程序 — MPU6050 姿态显示
├── empty.syscfg         # SysConfig 外设配置
├── hardware/
│   ├── delay.h/c        # delay_ms/delay_us (delay_cycles 忙等待)
│   ├── grey.h/c         # 灰度传感器串行接口驱动 (CLK+DAT)
│   ├── mpu6050.h/c      # MPU6050 驱动 (I2C 读写 + 姿态计算)
│   ├── mspm0_i2c.h/c    # MSPM0 硬件 I2C 封装 (DL_I2C, SDA 解锁)
│   ├── lcd.h/c          # ST7735S 绘图函数
│   ├── lcd_init.h/c     # ST7735S 软件 SPI 驱动 + 初始化
│   └── lcdfont.h        # ASCII 字库 (12×6 / 16×8 / 24×12 / 32×16)
├── Debug/               # 构建产物 (makefile, 自动生成)
└── targetConfigs/       # XDS110 调试配置
```

## SysConfig 外设一览

| 模块 | 实例名 | 用途 |
|------|--------|------|
| SYSCTL | — | 时钟树 (32 MHz) |
| I2C | I2C_MPU6050 | MPU6050 通信 (PA15/PA16, 400kHz) |
| GPIO | Grey | 灰度传感器 CLK 输出 (PA24) |
| GPIO | DAT | 灰度传感器 DAT 输入, 悬浮 (PA25) |
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
SYSCFG_DL_init()           ← SysConfig 生成的时钟+GPIO+I2C 初始化

main()
    ├── LCD_Init()         ← ST7735S 初始化 + 清屏
    ├── MPU6050_Init()     ← I2C SDA 解锁 + 唤醒 + 配置量程
    └── while(1)
        ├── MPU6050_Read() ← 读 14 字节原始数据 → 计算 Pitch/Roll/Yaw
        └── LCD 显示姿态角
            delay_ms(100)  ← 100ms (10Hz) 刷新
```

## SysConfig 注意事项

- `internalResistor` 合法值: `NONE`, `PULL_UP`, `PULL_DOWN` (没有 `PULL_DISABLE`)
- 生成宏名前缀: GPIO 实例用 `{name}_PORT/{name}_{pin}_PIN`, I2C 实例用 `GPIO_I2C_{name}_*`
- CCS 重新生成 makefile 后会覆盖手动编辑，新文件需重新添加到 `ORDERED_OBJS`

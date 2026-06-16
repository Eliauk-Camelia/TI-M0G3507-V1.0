# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

TI MSPM0G3507 (Cortex-M0+, 32MHz) 嵌入式项目，CCS Theia 70.4.0 + MSPM0 SDK 2.10.00.04 + SysConfig 1.26.2。
目标板：天孟星 MSPM0G3507 核心板 + 机器人扩展板。

## 构建命令

```bash
cd Debug && gmake -j$(nproc) all    # 构建
gmake -C Debug clean                 # 清理
dslite -c targetConfigs/MSPM0G3507.ccxml -e -r 2 -u Debug/TI-M0G3507-V1.0.out  # 烧录
```

工具链：`/opt/ccstudio/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang`，SDK：`/home/arch/ti/mspm0_sdk_2_10_00_04/`。**CLI 和 CCS 不要混用构建**——CCS 会重新生成 makefile 覆盖手动编辑。

新增 `.c` 到 `hardware/` 后需同步更新：
- `Debug/makefile` → `ORDERED_OBJS` 和 `clean` target
- `Debug/hardware/subdir_vars.mk` → `C_SRCS`/`OBJS`/`C_DEPS` 及 `__QUOTED` 变体

CPUCLK=32MHz，编译器优化 `-O2`，`-march=thumbv6m -mcpu=cortex-m0plus -mthumb`。

## 引脚映射（仅本项目）

### 调试

| 信号 | 引脚 |
|------|------|
| SWCLK | PA20 |
| SWDIO | PA19 |

### LCD ST7735S（软件 SPI，独立 GPIO 实例）

| 信号 | 引脚 | SysConfig 实例名 |
|------|------|-----------------|
| SCLK | PA31 | SCLK |
| MOSI | PA28 | MOSI |
| CS   | PA23 | CS |
| DC   | PB24 | DC |
| RES  | PB20 | RES |
| BLK  | PB19 | BLK (低电平点亮) |

禁止用 `LCD_A`/`LCD_B` 分组——必须独立 GPIO 实例，否则宏名不匹配。

### 灰度传感器（感为辅助板，串行 CLK+DAT）

| 信号 | 引脚 | 实例名 | 方向 |
|------|------|--------|------|
| CLK | PA24 | Grey (AD0) | 输出 |
| DAT | PA25 | DAT (PIN_25) | 输入 NONE (悬浮) |

### MPU6050（硬件 I2C，400kHz）

| 信号 | 引脚 | 实例名 |
|------|------|--------|
| SCL | PA15 | I2C_MPU6050 |
| SDA | PA16 | I2C_MPU6050 |

## SysConfig 外设一览

| 模块 | 实例名 | 用途 | 关键参数 |
|------|--------|------|---------|
| SYSCTL | — | 时钟树 32MHz | SYSOSC→PLL×2→÷2→÷2 |
| I2C | I2C_MPU6050 | MPU6050 (PA15/PA16) | 400kHz Fast Mode |
| TIMER | TIMER_0 | 1ms 周期中断 (TIMG0) | BUSCLK, prescale=8, ZERO 中断 |
| ADC12 | Grey_ADC | 灰度模拟输入 (PA27) | ULPCLK/8, 软件触发, 轮询 |
| GPIO | Grey (AD0) | 灰度 CLK 输出 (PA24) | — |
| GPIO | DAT (PIN_25) | 灰度 DAT 输入 (PA25) | 悬浮输入 |
| GPIO | BLK (PIN_19) | LCD 背光 (PB19) | 低电平点亮 |
| GPIO | CS (PIN_23) | LCD 片选 (PA23) | — |
| GPIO | DC (PIN_24) | LCD 数据/命令 (PB24) | — |
| GPIO | RES (PIN_20) | LCD 复位 (PB20) | — |
| GPIO | MOSI (PIN_28) | LCD 数据 (PA28) | — |
| GPIO | SCLK (PIN_31) | LCD 时钟 (PA31) | — |
| Board | — | 调试接口 (PA19/PA20) | SWD |

**ADC12 注意**：Grey_ADC (PA27) 已在 SysConfig 配置但当前应用代码未使用——灰度传感器走数字串行协议 (CLK+DAT)，ADC 为后续扩展预留。

## SysConfig 关键规则

- `empty.syscfg` 是外设配置唯一可编辑入口，构建时生成 `Debug/ti_msp_dl_config.c/h`——**不可手动编辑**
- `internalResistor` 合法值：`NONE`, `PULL_UP`, `PULL_DOWN`（没有 `PULL_DISABLE`）
- 生成宏名前缀：GPIO 实例用 `{name}_PORT`/`{name}_{pin}_PIN`，I2C 外设用 `GPIO_I2C_{name}_*`，TIMER 实例用 `{name}_INST`/`{name}_INST_INT_IRQN`
- SysConfig 每次运行覆盖 `device.opt`，需要持久化的宏定义写在源码里不要放 `device.opt`

## 源码结构

```
hardware/
├── delay.h/c        # delay_ms/delay_us (delay_cycles 忙等待)
├── lcd.h/c          # ST7735S 绘图 (Fill/Line/Circle/String/Int/Float)
├── lcd_init.h/c     # 软件 SPI 驱动 + ST7735S 初始化序列
├── lcdfont.h        # ASCII 字库 (12×6/16×8/24×12/32×16)
├── grey.h/c         # 灰度传感器 串行 CLK+DAT 协议
├── mpu6050.h/c      # MPU6050 DMP 驱动
├── mspm0_i2c.h/c    # 硬件 I2C 读写封装 (DL_I2C + SDA 解锁)
├── clock.h/c        # 毫秒时钟 (给 DMP 库的 delay/get_ms 回调)
├── inv_mpu.c/h      # Invensense MPU 库 (I2C 寄存器操作)
├── inv_mpu_dmp_motion_driver.c/h  # DMP 固件 + 200Hz 融合算法
├── dmpKey.h         # DMP 固件密钥
└── dmpmap.h         # DMP 内存映射
```

## 主循环架构（非阻塞定时器驱动）

`TIMER_0` (TIMG0) 提供 1ms 周期中断（BUSCLK, prescale=8, ZERO 匹配）：

```
TIMER_0 ISR (1ms)
  ├── g_tick++            ← 1ms 滴答计数
  ├── tick_ms = g_tick    ← 同步 clock.c（供 DMP 库 get_ms 回调）
  └── if (g_tick % 100 == 0) → g_refresh = 1   ← 100ms 刷新标志

main() 非阻塞循环
  ├── if (g_refresh) { MPU6050_Read(); LCD 刷新; }
  └── /* 其他非阻塞任务: 灰度/按键/串口等 */
```

**关键点**：
- `tick_ms` 声明在 `clock.c`，由 `empty.c` 的 ISR 更新——`mspm0_get_clock_ms()` 被 DMP 库内部调用获取时间戳
- `mspm0_delay_ms()` 直接调用忙等待 `delay_ms()`，不依赖 `tick_ms`（避免 DMP 初始化阶段 ISR 依赖问题）
- `g_refresh` 标志位模式确保 MPU6050 FIFO 读取和 LCD 刷新不会阻塞其他任务
- 如果 TIMER_0 配置有变动，需同步更新 `empty.syscfg` 中的 `timerPeriod`、`timerClkPrescale` 及 ISR 中的取模值（100ms = `period * N`）

## LCD 子系统（已验证的坑）

- 横屏 MADCTL = **0x78**（试过 0x38/0x68/0x28/0xA8/0xE8 都不对）
- `LCD_ShowChar` 像素写入用 **LSB 优先** (`temp & 0x01`, `>>= 1`)，因为 ST7735S GRAM 自增右→左
- **Mode=0 必须画背景像素** (`else if (!mode) LCD_DrawPoint(x,y,bc)`)，否则旧数据残留
- `LCD_Address_Set` 偏移量 +1(col) / +26(row) 是正确的，不要改
- `LCD_Writ_Bus` 需要 `delay_us(1)` 匹配 ST7735S 时序窗口

## 灰度传感器（感为辅助板 串行模式）

- 2 线协议：CLK 下降沿 → 读 DAT → CLK 上升沿 → 传感器更新下一位
- **首时钟丢弃**：先发一个 dummy 时钟，再读 8 位，否则数据右移一位
- `grey_read_digital()` 返回 8 位数字量 (bit0=探头1, ..., bit7=探头8)
- DAT 配置为悬浮输入 (`internalResistor = "NONE"`)，由辅助板驱动电平

## MPU6050 DMP 子系统

- I2C 地址 0x68 (AD0=GND)，400kHz Fast Mode
- 不使用 INT 中断引脚，轮询 `dmp_read_fifo()` 读取
- DMP 内部 200Hz 6 轴融合，MCU 100ms 读一次
- 宏定义写在源码里（`#define MOTION_DRIVER_TARGET_MSPM0` / `#define MPU6050`），不被 SysConfig 覆盖
- 启用功能：`6X_LP_QUAT | GYRO_CAL | SEND_CAL_GYRO | SEND_RAW_ACCEL`
- `GYRO_CAL`：上电静止 8 秒自动校准陀螺零偏
- `mspm0_delay_ms` → 直接调用 `delay_ms`（忙等待，不依赖定时器 ISR）
- SDA 解锁：`mpu6050_i2c_sda_unlock()` 切 GPIO 发 9 个 SCL 脉冲恢复死锁
- 四元数 → 欧拉角公式在 `MPU6050_Read()` 中
- `show_angle()` 处理负数显示（画 '-' 号 + 取绝对值）

## 关联参考项目

- `/home/arch/Projects/ti-mspm0/M0-V1.0/` — 工作项目（I2C 灰度 + 按键 + LED）
- `/home/arch/work-space/project-space/ProgramForCar-Main_Develop/` — MPU6050 DMP 参考
- `/home/arch/work-space/stm32-soace/感为灰度STM32F1xx标准库例程/` — 灰度传感器 STM32 例程

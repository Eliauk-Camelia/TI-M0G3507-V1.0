# 感为灰度传感器在 MSPM0G3507 上的三种驱动方式

## 前言

最近在做一个智能车相关的项目，用到了**感为（GW）灰度传感器**。这款传感器集成了 8 路红外探头，常用于循迹小车的赛道检测。项目的主控换成了 TI 的 **MSPM0G3507**（Cortex-M0+, 80MHz），这在嵌入式圈子里还算比较新的片子。本文记录一下从零开始在 MSPM0 平台上驱动这款传感器的完整过程。

## 硬件概览

| 项目 | 说明 |
|------|------|
| **MCU** | MSPM0G3507 (Cortex-M0+, 32 MHz) |
| **开发板** | LP_MSPM0G3507 LaunchPad |
| **传感器** | 感为灰度传感器 (8 路探头) |
| **LCD** | ST7735S (160×80, SPI) |
| **工具链** | CCS Theia + TI Arm Clang |

感为灰度传感器的接口比较灵活，支持三种方式输出数据：

1. **模拟电压** — 每个探头输出 0~3.3V 的模拟电压，需要外部 ADC 采集
2. **串行数字** — 2 线 CLK + DAT，8 个时钟读回 8 位数字量
3. **I2C 数字** — 标准 I2C 从机 (地址 0x4C)，可读数字量和模拟量

下面逐一介绍三种方案的实现。


## 方案一：模拟电压 + 模拟开关 (ADC)

这是最早期的方案。传感器 8 路探头的模拟电压输出，经过一片 **CD4051**（8 选 1 模拟开关）后接到 MSPM0 的 ADC 引脚 (PA27)。

### 引脚连接

| 功能 | 引脚 | 说明 |
|------|------|------|
| ADC_IN | PA27 | ADC0 模拟输入 |
| AD0 | PA24 | 模拟开关地址线 bit0 |
| AD1 | PA25 | 模拟开关地址线 bit1 |
| AD2 | PA26 | 模拟开关地址线 bit2 |

### 软件实现

核心逻辑很直接——切换模拟开关通道，触发一次 ADC 转换，轮询等待完成，读取结果：

```c
static void toggle_mux(uint8_t channel)
{
    // 清空地址线，消除上一通道的电平残留
    DL_GPIO_clearPins(Grey_PORT, Grey_AD0_PIN | Grey_AD1_PIN | Grey_AD2_PIN);
    // 按 channel 设置 AD0~AD2
    // 延时等待模拟开关稳定
    delay_cycles(1500);
}

static uint16_t read_adc(void)
{
    DL_ADC12_startConversion(Grey_ADC_INST);
    while (DL_ADC12_getStatus(Grey_ADC_INST) & DL_ADC12_STATUS_CONVERSION_ACTIVE);
    return DL_ADC12_getMemResult(Grey_ADC_INST, Grey_ADC_ADCMEM_0);
}

void read_all_adc(uint16_t out_buf[8])
{
    for (int i = 0; i < 8; i++) {
        toggle_mux(i);
        out_buf[i] = read_adc();
    }
}
```

> **★ Insight ─────────────────────────────────────**
> `DL_ADC12_startConversion()` 内部同时置位 `SC`（开始转换）和 `ENC`（使能转换）位，一步完成触发。TI 的 DriverLib 封装比直接写寄存器更安全——比如它会保证 ENC 只在 SC 为低时才能被置位，避免了硬件手册里那句容易踩坑的 "ADC12CTL0.ENC must be toggled from 0 to 1 while ADC12CTL0.SC=0"。
> `─────────────────────────────────────────────────`

### 优缺点

- **优点**：可以得到 0~4095 的原始 ADC 值，灰度分辨力最高
- **缺点**：需要额外一片 CD4051 做通道切换，每读一轮 8 通道要做 8 次 ADC 转换和 8 次模拟开关切换，速度较慢（约 1~2ms/轮），且多占用 3 个 MCU 引脚做地址线


## 方案二：串行数字协议 (2 线 CLK + DAT)

后来发现，感为灰度传感器的配套辅助板直接提供串行数字输出接口——只需要 **CLK** 和 **DAT** 两根线，8 个时钟周期就能读完 8 个探头的数据。

### 协议时序

```
CLK  ─┐     ┌──┐  ┌──┐  ┌──┐       ┌──
      └─────┘  └──┘  └──┘  └─ ... ─┘
            ↓      ↓      ↓
DAT  ───────X──────X──────X── ... ────
            bit0   bit1   bit2
```

- CLK 默认高电平，空闲状态
- CLK 下降沿 → 传感器锁存当前探头数据到 DAT 线
- MCU 读 DAT → 得到当前位
- CLK 上升沿 → 传感器准备下一位
- 8 个周期后，完整 8 位数据 (`bit0=探头1, bit7=探头8`)

### 软件实现

```c
uint8_t grey_read_digital(void)
{
    uint8_t ret = 0;

    // 首时钟丢弃：辅助板首周期数据不稳定
    DL_GPIO_clearPins(CLK_PORT, CLK_PIN);
    serial_delay();
    DL_GPIO_setPins(CLK_PORT, CLK_PIN);
    serial_delay();

    for (uint8_t i = 0; i < 8; i++) {
        DL_GPIO_clearPins(CLK_PORT, CLK_PIN);  // 下降沿
        serial_delay();
        if (DL_GPIO_readPins(DAT_PORT, DAT_PIN_25_PIN))
            ret |= (1 << i);                     // 读 DAT
        DL_GPIO_setPins(CLK_PORT, CLK_PIN);     // 上升沿
        serial_delay();
    }
    return ret;
}
```

### 初始化时序

传感器上电后默认处于模拟电压输出模式，需要发一段特殊时序唤醒串行模式：

```c
void grey_init(void)
{
    DL_GPIO_setPins(CLK_PORT, CLK_PIN);
    delay_cycles(3200);   // 100μs
    DL_GPIO_clearPins(CLK_PORT, CLK_PIN);
    delay_cycles(3200);   // 100μs
    DL_GPIO_setPins(CLK_PORT, CLK_PIN);
    delay_cycles(32000);  // 1ms 等待稳定
}
```

> **★ Insight ─────────────────────────────────────**
> 为什么需要"丢弃第一个时钟"？传感器上电后内部移位寄存器的初始状态不确定——可能残留了上次掉电时的随机值。先发一个空时钟让移位寄存器完成第一次真正的数据锁存，之后 8 个时钟的数据才是当前探头状态的可靠反映。这是从感为官方 STM32 参考例程里学到的经验，移植时保留了这个"看起来多余但实际必要的"操作。
> `─────────────────────────────────────────────────`

### 优缺点

- **优点**：只需 2 个 GPIO，速度极快（~80μs 读完 8 路），不需要额外硬件
- **缺点**：只能读数字量（0/1），没有灰度值；需要 MCU 定时去"拉"数据，不能中断通知


## 方案三：I2C 数字协议（最推荐的方案）

最终方案是通过 I2C 总线读取传感器数据。感为灰度传感器作为 I2C 从机（地址 `0x4C`），内部有三个关键寄存器：

| 寄存器 | 地址 | 说明 |
|--------|------|------|
| Ping | `0xAA` | 返回 `0x66` 表示通信正常 |
| Digital | `0xDD` | 返回 1 字节数字量 (8 探头) |
| Analog | `0xB0` | 返回 8 字节模拟量 (0~255) |

### 核心挑战：MSPM0 没有硬件开漏

标准 I2C 总线要求 SDA 和 SCL 都是**开漏 + 外部上拉**——主机可以主动拉低，释放后由外部上拉电阻拉高。但 MSPM0 的 GPIO 不支持硬件开漏模式，怎么办？

### 解决思路：GPIO 方向切换模拟开漏

通过**动态切换 GPIO 方向**来模拟开漏行为：

```
输出 "0" → 设为输出模式 + 清引脚 (强下拉)
输出 "1" → 设为输入模式 (高阻态) → 外部上拉电阻拉高
读取    → 设为输入模式 → 读引脚电平
```

具体实现如下——这是整个项目中最核心的设计：

```c
// SDA 输出：通过切换 GPIO 方向模拟开漏
static void gray_sda_out(uint8_t bit, void *user_data)
{
    (void)user_data;
    if (bit) {
        // 释放总线：切输入（高阻）→ 外部上拉自动拉高
        DL_GPIO_initDigitalInput(GRAY_SDA_PIN_16_IOMUX);
    } else {
        // 拉低：切输出 + 清引脚（强下拉）
        DL_GPIO_initDigitalOutput(GRAY_SDA_PIN_16_IOMUX);
        DL_GPIO_clearPins(GRAY_SDA_PORT, GRAY_SDA_PIN_16_PIN);
    }
    delay_us(5);
}

// SDA 输入：读 ACK/数据
static uint8_t gray_sda_in(void *user_data)
{
    (void)user_data;
    DL_GPIO_initDigitalInput(GRAY_SDA_PIN_16_IOMUX);
    return DL_GPIO_readPins(GRAY_SDA_PORT, GRAY_SDA_PIN_16_PIN) ? 1 : 0;
}

// SCL 始终为输出（主机控制时钟）
static void gray_scl_out(uint8_t bit, void *user_data)
{
    (void)user_data;
    if (bit)
        DL_GPIO_setPins(GRAY_SCL_PORT, GRAY_SCL_PIN_15_PIN);
    else
        DL_GPIO_clearPins(GRAY_SCL_PORT, GRAY_SCL_PIN_15_PIN);
    delay_us(5);
}
```

> **★ Insight ─────────────────────────────────────**
> 用 GPIO 方向切换模拟开漏，有一个容易被忽略的细节：**切换方向时不能先设输出再写电平**——那会在切换瞬间输出一个不确定的值。正确的顺序是：先切方向为输出（此时输出寄存器默认值可能是高也可能是低），然后**立即**写目标电平。MSPM0 的 `DL_GPIO_initDigitalOutput()` 之后紧接着 `DL_GPIO_clearPins()` 就是干这个的，两个操作之间不过几十个 CPU 周期，对 I2C 的 100kHz 时序来说完全可忽略。但如果在两行之间插入了 `delay_us()`，总线就会短暂出现一个不确定的电平。
> `─────────────────────────────────────────────────`

### 通用软件 I2C 引擎

为了更好的代码复用，这部分被抽象成了一个**回调模式的通用软件 I2C 驱动**（`sw_i2c.c`）。设计思路：

```c
typedef struct {
    void     (*sda_out)(uint8_t bit, void *user_data);  // SDA 输出回调
    uint8_t  (*sda_in) (void *user_data);                // SDA 输入回调
    void     (*scl_out)(uint8_t bit, void *user_data);   // SCL 输出回调
    void     *user_data;                                  // 用户上下文
} sw_i2c_interface_t;
```

上层（传感器驱动）只需实现这三个回调函数，填好接口表，剩下的 I2C 时序——Start/Stop/Write/Read/MemRead——全部由 `sw_i2c` 引擎自动完成。这种设计的好处是：

- **平台无关**：只依赖回调函数，不碰任何寄存器，换个 MCU 换个引脚都不需要改 I2C 引擎
- **传感器无关**：换一个 I2C 传感器，只需调整地址和寄存器定义
- **可测试**：理论上可以把回调换成模拟函数来单元测试（虽然在嵌入式里单测不多见）

`sw_i2c_mem_read()` 的实现完全是教科书式的 I2C 复合读写序列：

```c
uint8_t sw_i2c_mem_read(sw_i2c_interface_t *i2c, uint8_t addr,
                        uint8_t mem_addr, uint8_t *data, uint8_t len)
{
    // 第一步：Start → 写地址 → 写寄存器地址
    i2c_start(i2c);
    i2c_write_byte(i2c, addr & 0xFE);
    i2c_write_byte(i2c, mem_addr);

    // 第二步：重复 Start → 读地址 → 读数据 → Stop
    i2c_start(i2c);                              // 重复起始条件
    i2c_write_byte(i2c, addr | 0x01);
    for (uint8_t i = 0; i < len; i++) {
        data[i] = i2c_read_byte(i2c, i < (len - 1));  // 最后一字节 NACK
    }
    i2c_stop(i2c);
    return 0;
}
```

### 传感器驱动

有了 I2C 引擎，传感器驱动就非常简洁了：

```c
// 初始化：发 Ping 直到握手成功
int gray_init(void)
{
    uint8_t rsp = 0;
    while (rsp != GW_GRAY_PING_OK) {
        sw_i2c_mem_read(&i2c_if, GW_GRAY_ADDR_R,
                        GW_GRAY_PING, &rsp, 1);
        delay_ms(500);
    }
    return 0;
}

// 读数字量
int gray_read_digital(uint8_t *digital)
{
    return sw_i2c_mem_read(&i2c_if, GW_GRAY_ADDR_R,
                           GW_GRAY_DIGITAL_MODE, digital, 1);
}

// 读模拟量 (0~255)
int gray_read_analog(uint8_t analog[8])
{
    return sw_i2c_mem_read(&i2c_if, GW_GRAY_ADDR_R,
                           GW_GRAY_ANALOG_MODE, analog, 8);
}
```

### 优缺点

- **优点**：同时支持数字量和模拟量（0~255），只需 2 个 GPIO (SCL + SDA)，速度适中（~1ms 读完 8 通道模拟量），支持 ACK 应答检测通信错误
- **缺点**：软件 I2C 占 CPU 时间（while 等待），不能和硬件 I2C 相比的速率；GPIO 方向切换的开漏模拟在极限温度下可能时序变差


## 三种方案对比总结

| 维度 | 方案一：ADC+模拟开关 | 方案二：串行数字 | 方案三：I2C |
|------|---------------------|-----------------|------------|
| 数据精度 | 12-bit (0~4095) | 1-bit (0/1) | 8-bit 模拟 + 1-bit 数字 |
| MCU 引脚 | 4 (ADC + 3 地址线) | 2 (CLK + DAT) | 2 (SCL + SDA) |
| 额外硬件 | CD4051 模拟开关 | 无 | 外部上拉电阻 |
| 读一轮速度 | ~2ms | ~80μs | ~1ms |
| 通信可靠性 | 高（ADC 硬件） | 中（无应答） | 高（ACK 检测） |
| 实现复杂度 | 低 | 低 | 中（需 I2C 引擎） |

## 完整系统结构

在实际项目中，整个系统的运行流程如下：

```
SYSCFG_DL_init()           ← SysConfig 自动生成外设初始化
    ├── 时钟 (32 MHz CPUCLK)
    ├── GPIO (传感器 + LCD 控制线)
    ├── ADC12 (保留)
    └── TIMER_0 (1ms 系统滴答)

main()
    ├── LCD_Init()         ← ST7735S 软件 SPI 初始化
    ├── grey_init()        ← 传感器握手
    └── while(1) {
           读取传感器数据 → LCD 显示 → delay_ms(200)
        }
```

项目文件结构：

```
hardware/
├── grey.c / grey.h        ← 串行数字协议驱动
├── gray.c / gray.h        ← I2C 协议驱动 (最新方案)
├── sw_i2c.c / sw_i2c.h    ← 通用软件 I2C 引擎
├── lcd.c / lcd.h          ← LCD 绘图函数
├── lcd_init.c / lcd_init.h ← LCD 软件 SPI 驱动
├── lcdfont.h              ← 字库
└── delay.c / delay.h      ← 延时函数
```

## 后记

从 ADC + CD4051 的原始方案，到 2 线串行协议，再到最终的软件 I2C 方案，整个过程其实是一个**逐步化繁为简**的过程。

最有意思的是 GPIO 方向切换模拟开漏这个 trick——虽然硬件手册上告诉你 M0 没有开漏，但实际上换个角度想，"输入高阻"配合外部上拉电阻就是开漏的 "1" 状态。写嵌入式代码的乐趣之一，正是在这种"用软件填补硬件缺口"的过程里。

如果你也在用 MSPM0 做传感器驱动，欢迎交流讨论。

---

*硬件平台：LP_MSPM0G3507 LaunchPad | 编译器：TI Arm Clang 4.0.4 | 日期：2026-06-16*

#include "ti_msp_dl_config.h"
#include "grey.h"

/**
 * @brief 切换多路模拟开关3位地址线（AD2 AD1 AD0）  PA24 25 26 
 * @param channel 目标通道 channel_1 ~ channel_8
 */
static void __toggle_GPIO(CHANNEL_t channel)
{
    // 先清空全部地址线，消除上一通道电平残留
    DL_GPIO_clearPins(Grey_PORT, Grey_AD0_PIN | Grey_AD1_PIN | Grey_AD2_PIN);

    switch (channel) {
        case channel_1: // 000
            break;
        case channel_2: // 001 AD0=1
            DL_GPIO_writePins(Grey_PORT, Grey_AD0_PIN);
            break;
        case channel_3: // 010 AD1=1
            DL_GPIO_writePins(Grey_PORT, Grey_AD1_PIN);
            break;
        case channel_4: // 011 AD0+AD1=1
            DL_GPIO_writePins(Grey_PORT, Grey_AD0_PIN | Grey_AD1_PIN);
            break;
        case channel_5: // 100 AD2=1
            DL_GPIO_writePins(Grey_PORT, Grey_AD2_PIN);
            break;
        case channel_6: // 101 AD2+AD0=1
            DL_GPIO_writePins(Grey_PORT, Grey_AD2_PIN | Grey_AD0_PIN);
            break;
        case channel_7: // 110 AD2+AD1=1
            DL_GPIO_writePins(Grey_PORT, Grey_AD2_PIN | Grey_AD1_PIN);
            break;
        case channel_8: // 111 AD2+AD1+AD0=1
            DL_GPIO_writePins(Grey_PORT, Grey_AD2_PIN | Grey_AD1_PIN | Grey_AD0_PIN);
            break;
        default:
            // 非法通道，保持全部低电平
            break;
    }
    // 增加建立延时，模拟开关电压稳定（DriverLib 延时接口）
    delay_cycles(1500);
}

/**
 * @brief 单次读取ADC原始12位采样值 (PA27)
 * @return 0~4095 ADC采样结果
 *
 * @note 使用新版 DL_ADC12_* API，Grey_ADC_INST / Grey_ADC_ADCMEM_0 由 SysConfig 生成。
 *       DL_ADC12_startConversion() 内置 SC+ENC 位，轮询 STATUS 寄存器等待转换完成。
 */
static uint16_t __read_adc(void)            // PA27
{
    DL_ADC12_startConversion(Grey_ADC_INST);
    /* 轮询等待转换完成 (BUSY 位清零) */
    while (DL_ADC12_getStatus(Grey_ADC_INST) & DL_ADC12_STATUS_CONVERSION_ACTIVE);
    uint16_t adcRaw = DL_ADC12_getMemResult(Grey_ADC_INST, Grey_ADC_ADCMEM_0);
    return adcRaw;
}

/**
 * @brief 读取全部8路模拟开关ADC通道
 * @param out_buf 输出数组，长度≥8，存储8通道采样值
 * @note 替代返回静态数组方案，消除重入/数据覆盖风险
 */
void read_all_adc(uint16_t out_buf[8])
{
    for (CHANNEL_t i = channel_1; i < channel_ALL; i++)
    {
        __toggle_GPIO(i);
        out_buf[i] = __read_adc();
    }
}

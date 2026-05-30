/**
 * @file    adc_driver.h
 * @brief   STM32 ADC 单通道轮询与多通道 DMA 扫描驱动
 */
#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "main.h"

#define ADC_CHANNELS 3U

uint16_t ADC_Read(void);
float    ADC_ReadVoltage(void);

void     ADC_DMA_Start(void);
uint8_t  ADC_DMA_SampleBlock(void);
uint32_t ADC_GetChannelMilliVolt(uint8_t ch);
float    ADC_GetChannelVoltage(uint8_t ch);
uint8_t  ADC_IsReady(void);
void     ADC_ClearReady(void);

#endif /* ADC_DRIVER_H */

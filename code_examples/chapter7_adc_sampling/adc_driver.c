/**
 * @file    adc_driver.c
 * @brief   三通道 DMA：HAL_ADC_Start_DMA + 轮询 TC/EOC（兼容 PicSimLab 无 DMA 中断）
 */
#include "adc_driver.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

static const uint32_t adc_ch_list[ADC_CHANNELS] = {
    ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2
};

static uint16_t         adc_buf[ADC_CHANNELS];
static volatile uint8_t adc_ready = 0;

uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return 0;
    }
    uint16_t value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return value;
}

float ADC_ReadVoltage(void)
{
    return (float)ADC_Read() * 3.3f / 4095.0f;
}

static uint8_t ADC_DMA_WaitDone(uint8_t idx)
{
    uint32_t t0 = HAL_GetTick();
    uint32_t tc_flag = __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_adc1);

    while ((HAL_GetTick() - t0) < 1000U) {
        if (adc_ready) {
            return 1;
        }

        /* qemu-stm32 常不触发 NVIC，轮询 TC 后手动走 HAL DMA 完成路径 */
        if (__HAL_DMA_GET_FLAG(&hdma_adc1, tc_flag)) {
            __HAL_DMA_CLEAR_FLAG(&hdma_adc1, tc_flag);
            HAL_DMA_IRQHandler(&hdma_adc1);
            if (adc_ready) {
                return 1;
            }
        }

        /* 后备：ADC 已 EOC 但 DMA 中断未仿真，读 DR 填入缓冲区 */
        if (__HAL_ADC_GET_FLAG(&hadc1, ADC_FLAG_EOC)) {
            adc_buf[idx] = (uint16_t)HAL_ADC_GetValue(&hadc1);
            __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC);
            (void)HAL_ADC_Stop_DMA(&hadc1);
            adc_ready = 1;
            return 1;
        }
    }

    (void)HAL_ADC_Stop_DMA(&hadc1);
    return 0;
}

static uint8_t ADC_DMA_ReadOne(uint8_t idx)
{
    ADC_ChannelConfTypeDef s = {0};

    s.Channel = adc_ch_list[idx];
    s.Rank = ADC_REGULAR_RANK_1;
    s.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &s) != HAL_OK) {
        return 0;
    }

    adc_ready = 0;
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&adc_buf[idx], 1) != HAL_OK) {
        return 0;
    }

    return ADC_DMA_WaitDone(idx);
}

void ADC_DMA_Start(void)
{
    (void)ADC_DMA_SampleBlock();
}

uint8_t ADC_DMA_SampleBlock(void)
{
    for (uint8_t i = 0; i < ADC_CHANNELS; i++) {
        if (!ADC_DMA_ReadOne(i)) {
            return 0;
        }
    }
    return 1;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        adc_ready = 1;
    }
}

uint32_t ADC_GetChannelMilliVolt(uint8_t ch)
{
    if (ch >= ADC_CHANNELS) {
        return 0;
    }
    return (uint32_t)adc_buf[ch] * 3300U / 4095U;
}

float ADC_GetChannelVoltage(uint8_t ch)
{
    return (float)ADC_GetChannelMilliVolt(ch) / 1000.0f;
}

uint8_t ADC_IsReady(void)
{
    return adc_ready;
}

void ADC_ClearReady(void)
{
    adc_ready = 0;
}

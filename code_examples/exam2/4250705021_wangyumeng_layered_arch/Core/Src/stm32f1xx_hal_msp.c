/**
 * @file    stm32f1xx_hal_msp.c
 * @brief   MSP Initialization and de-Initialization codes.
 */

#include "main.h"

/**
 * @brief  初始化全局 MSP
 */
void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

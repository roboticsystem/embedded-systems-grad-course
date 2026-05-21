/**
 * @file    gpio.c
 * @brief   GPIO 初始化（CubeMX 生成风格）
 * @note    配置 PC13 为推挽输出（板载 LED）
 */

#include "gpio.h"

/**
 * @brief  GPIO 初始化函数
 * @note   配置 PC13 为推挽输出，初始高电平（LED 熄灭）
 */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能 GPIOC 时钟 */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 配置 PC13：推挽输出，无上拉/下拉，高速 */
    GPIO_InitStruct.Pin   = GPIO_PIN_13;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* 初始输出高电平（LED 熄灭，PC13 低电平有效） */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

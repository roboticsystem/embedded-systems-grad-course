/**
 * @file    gpio.c
 * @brief   GPIO 初始化（CubeMX 生成风格）
 * @note    配置 PC13（板载 LED）与 PA0（扩展 LED）为推挽输出
 */

#include "gpio.h"
#include "main.h"

/**
 * @brief  GPIO 初始化函数
 * @note   PC13：板载 LED，低电平点亮，初始置高（熄灭）
 *         PA0 ：扩展 LED，初始置低（熄灭）
 */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能 GPIOA / GPIOC 时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 初始电平：PC13 高（熄灭），PA0 低（熄灭） */
    HAL_GPIO_WritePin(LED_ONBOARD_GPIO_Port, LED_ONBOARD_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_EXT_GPIO_Port, LED_EXT_Pin, GPIO_PIN_RESET);

    /* 配置 PC13：推挽输出，无上拉/下拉，高速 */
    GPIO_InitStruct.Pin   = LED_ONBOARD_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_ONBOARD_GPIO_Port, &GPIO_InitStruct);

    /* 配置 PA0：推挽输出，无上拉/下拉，高速 */
    GPIO_InitStruct.Pin   = LED_EXT_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LED_EXT_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @file    hal_gpio.h
 * @brief   硬件抽象层 — GPIO 接口定义
 * @note    平台无关，不包含任何芯片特定头文件
 *          移植到其他平台时，仅需替换 hal_gpio_stm32.c 的实现
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

/**
 * @brief GPIO 引脚状态枚举
 */
typedef enum {
    GPIO_PIN_RESET = 0,   /**< 引脚低电平 */
    GPIO_PIN_SET   = 1    /**< 引脚高电平 */
} GPIO_PinState;

/**
 * @brief  设置 GPIO 引脚输出电平
 * @param  port  端口索引（0=GPIOA, 1=GPIOB, 2=GPIOC, ...）
 * @param  pin   引脚编号（0~15）
 * @param  state 目标电平（GPIO_PIN_RESET 或 GPIO_PIN_SET）
 */
void hal_gpio_write(uint8_t port, uint8_t pin, GPIO_PinState state);

/**
 * @brief  翻转 GPIO 引脚电平
 * @param  port  端口索引
 * @param  pin   引脚编号
 */
void hal_gpio_toggle(uint8_t port, uint8_t pin);

/**
 * @brief  读取 GPIO 引脚当前电平
 * @param  port  端口索引
 * @param  pin   引脚编号
 * @return 引脚电平状态
 */
GPIO_PinState hal_gpio_read(uint8_t port, uint8_t pin);

#endif /* HAL_GPIO_H */

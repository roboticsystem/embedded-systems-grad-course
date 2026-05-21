/**
 * @file    hal_gpio_stm32.c
 * @brief   HAL GPIO 接口的 STM32F1 平台实现
 * @note    仅此文件依赖 STM32 HAL 库，移植到其他平台时替换此文件即可
 */

#include "hal_gpio.h"
#include "stm32f1xx_hal.h"

/**
 * @brief 端口索引到 GPIO_TypeDef 指针的映射表
 * @note  port=0 → GPIOA, port=1 → GPIOB, port=2 → GPIOC
 *        如需更多端口，在此表中追加
 */
static GPIO_TypeDef* port_map[] = {
    GPIOA, GPIOB, GPIOC
};

#define PORT_MAP_SIZE  (sizeof(port_map) / sizeof(port_map[0]))

void hal_gpio_write(uint8_t port, uint8_t pin, GPIO_PinState state)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        HAL_GPIO_WritePin(port_map[port], (1U << pin), state);
    }
}

void hal_gpio_toggle(uint8_t port, uint8_t pin)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        HAL_GPIO_TogglePin(port_map[port], (1U << pin));
    }
}

GPIO_PinState hal_gpio_read(uint8_t port, uint8_t pin)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        return (GPIO_PinState)HAL_GPIO_ReadPin(port_map[port], (1U << pin));
    }
    return GPIO_PIN_RESET;
}

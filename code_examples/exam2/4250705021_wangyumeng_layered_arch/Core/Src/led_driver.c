/**
 * @file    led_driver.c
 * @brief   LED 驱动层实现
 * @note    基于 HAL GPIO 接口封装，平台无关
 *          仅需修改 led_driver.h 中的 LED_PORT/LED_PIN 即可适配不同硬件
 */

#include "led_driver.h"
#include "hal_gpio.h"

void led_on(void)
{
    /* PC13 LED 低电平点亮 */
    hal_gpio_write(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

void led_off(void)
{
    /* PC13 LED 高电平熄灭 */
    hal_gpio_write(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void led_toggle(void)
{
    hal_gpio_toggle(LED_PORT, LED_PIN);
}

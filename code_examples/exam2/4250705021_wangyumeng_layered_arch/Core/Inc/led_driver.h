/**
 * @file    led_driver.h
 * @brief   LED 驱动层 — 基于 HAL GPIO 接口封装
 * @note    修改 LED_PORT/LED_PIN 即可适配不同硬件，无需修改驱动逻辑
 */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

/**
 * @brief LED 硬件连接配置
 * @note  LED_PORT: 端口索引（0=GPIOA, 1=GPIOB, 2=GPIOC）
 *        LED_PIN:  引脚编号（0~15）
 *        修改此处即可适配不同硬件，无需修改驱动逻辑
 */
#define LED_PORT    2       /**< GPIOC（port_map 索引 2） */
#define LED_PIN     13      /**< PC13 引脚 */

/**
 * @brief  点亮 LED
 * @note   PC13 板载 LED 低电平点亮（低电平有效）
 */
void led_on(void);

/**
 * @brief  熄灭 LED
 */
void led_off(void);

/**
 * @brief  翻转 LED 状态
 */
void led_toggle(void);

#endif /* LED_DRIVER_H */

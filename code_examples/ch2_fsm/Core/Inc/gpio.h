/**
  ******************************************************************************
  * @file           : gpio.h
  * @brief          : GPIO驱动接口定义
  * @author         : Zhou Qing
  * @date           : 2024
  ******************************************************************************
  */

#ifndef __GPIO_H
#define __GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* 引脚定义 */
#define LED_LOCKED_PIN    GPIO_PIN_0
#define LED_UNLOCKED_PIN  GPIO_PIN_1
#define LED_ALARM_PIN     GPIO_PIN_2
#define BUZZER_PIN        GPIO_PIN_3

#define LED_PORT          GPIOB
#define BUZZER_PORT       GPIOB

#define KEY_PW0_PIN       GPIO_PIN_0
#define KEY_PW1_PIN       GPIO_PIN_1
#define KEY_PW2_PIN       GPIO_PIN_2
#define KEY_PORT          GPIOA

/* 函数声明 */
void gpio_init(void);
void led_locked_on(void);
void led_locked_off(void);
void led_unlocked_on(void);
void led_unlocked_off(void);
void led_alarm_on(void);
void led_alarm_off(void);
void buzzer_on(void);
void buzzer_off(void);
uint8_t key_pw0_read(void);
uint8_t key_pw1_read(void);
uint8_t key_pw2_read(void);
void update_leds(LockState state);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H */
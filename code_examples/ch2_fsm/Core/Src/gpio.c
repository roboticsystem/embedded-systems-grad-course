/**
  ******************************************************************************
  * @file           : gpio.c
  * @brief          : GPIO驱动实现
  * @author         : Zhou Qing
  * @date           : 2024
  ******************************************************************************
  */

#include "gpio.h"
#include "stm32f4xx_hal.h"

/**
  * @brief  初始化GPIO
  * @param  无
  * @retval 无
  */
void gpio_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能GPIOA时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* 使能GPIOB时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 配置PA0-PA2为输入模式（按键） */
    GPIO_InitStruct.Pin = KEY_PW0_PIN | KEY_PW1_PIN | KEY_PW2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY_PORT, &GPIO_InitStruct);

    /* 配置PB0-PB3为输出模式（LED和蜂鸣器） */
    GPIO_InitStruct.Pin = LED_LOCKED_PIN | LED_UNLOCKED_PIN | LED_ALARM_PIN | BUZZER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    /* 初始状态：所有LED关闭，蜂鸣器关闭 */
    led_locked_off();
    led_unlocked_off();
    led_alarm_off();
    buzzer_off();
}

/**
  * @brief  点亮锁定状态LED
  * @param  无
  * @retval 无
  */
void led_locked_on(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_LOCKED_PIN, GPIO_PIN_SET);
}

/**
  * @brief  熄灭锁定状态LED
  * @param  无
  * @retval 无
  */
void led_locked_off(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_LOCKED_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  点亮解锁状态LED
  * @param  无
  * @retval 无
  */
void led_unlocked_on(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_UNLOCKED_PIN, GPIO_PIN_SET);
}

/**
  * @brief  熄灭解锁状态LED
  * @param  无
  * @retval 无
  */
void led_unlocked_off(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_UNLOCKED_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  点亮报警状态LED
  * @param  无
  * @retval 无
  */
void led_alarm_on(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_ALARM_PIN, GPIO_PIN_SET);
}

/**
  * @brief  熄灭报警状态LED
  * @param  无
  * @retval 无
  */
void led_alarm_off(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_ALARM_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  开启蜂鸣器
  * @param  无
  * @retval 无
  */
void buzzer_on(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

/**
  * @brief  关闭蜂鸣器
  * @param  无
  * @retval 无
  */
void buzzer_off(void) {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  读取密码按键0
  * @param  无
  * @retval 按键状态（0=未按下，1=按下）
  */
uint8_t key_pw0_read(void) {
    return (uint8_t)(!HAL_GPIO_ReadPin(KEY_PORT, KEY_PW0_PIN));
}

/**
  * @brief  读取密码按键1
  * @param  无
  * @retval 按键状态（0=未按下，1=按下）
  */
uint8_t key_pw1_read(void) {
    return (uint8_t)(!HAL_GPIO_ReadPin(KEY_PORT, KEY_PW1_PIN));
}

/**
  * @brief  读取密码按键2
  * @param  无
  * @retval 按键状态（0=未按下，1=按下）
  */
uint8_t key_pw2_read(void) {
    return (uint8_t)(!HAL_GPIO_ReadPin(KEY_PORT, KEY_PW2_PIN));
}

/**
  * @brief  根据状态更新LED显示
  * @param  state: 当前状态
  * @retval 无
  */
void update_leds(LockState state) {
    /* 先关闭所有LED */
    led_locked_off();
    led_unlocked_off();
    led_alarm_off();
    
    /* 根据状态点亮对应LED */
    switch (state) {
        case STATE_LOCKED:
            led_locked_on();
            break;
        case STATE_UNLOCKED:
            led_unlocked_on();
            break;
        case STATE_OPEN:
            led_unlocked_on();  /* 门打开时保持解锁状态指示 */
            break;
        case STATE_ALARM:
            led_alarm_on();
            break;
        default:
            break;
    }
}
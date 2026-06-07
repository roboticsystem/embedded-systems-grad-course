/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_LOCKED_Pin|LED_UNLOCKED_Pin|LED_ALARM_Pin|BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : KEY_PW0_Pin KEY_PW1_Pin KEY_PW2_Pin */
  GPIO_InitStruct.Pin = KEY_PW0_Pin|KEY_PW1_Pin|KEY_PW2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_LOCKED_Pin LED_UNLOCKED_Pin LED_ALARM_Pin BUZZER_Pin */
  GPIO_InitStruct.Pin = LED_LOCKED_Pin|LED_UNLOCKED_Pin|LED_ALARM_Pin|BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

void led_locked_on(void)
{
  HAL_GPIO_WritePin(LED_LOCKED_GPIO_Port, LED_LOCKED_Pin, GPIO_PIN_SET);
}

void led_locked_off(void)
{
  HAL_GPIO_WritePin(LED_LOCKED_GPIO_Port, LED_LOCKED_Pin, GPIO_PIN_RESET);
}

void led_unlocked_on(void)
{
  HAL_GPIO_WritePin(LED_UNLOCKED_GPIO_Port, LED_UNLOCKED_Pin, GPIO_PIN_SET);
}

void led_unlocked_off(void)
{
  HAL_GPIO_WritePin(LED_UNLOCKED_GPIO_Port, LED_UNLOCKED_Pin, GPIO_PIN_RESET);
}

void led_alarm_on(void)
{
  HAL_GPIO_WritePin(LED_ALARM_GPIO_Port, LED_ALARM_Pin, GPIO_PIN_SET);
}

void led_alarm_off(void)
{
  HAL_GPIO_WritePin(LED_ALARM_GPIO_Port, LED_ALARM_Pin, GPIO_PIN_RESET);
}

void buzzer_on(void)
{
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
}

void buzzer_off(void)
{
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

uint8_t key_pw0_read(void)
{
  return (uint8_t)(HAL_GPIO_ReadPin(KEY_PW0_GPIO_Port, KEY_PW0_Pin) == GPIO_PIN_RESET);
}

uint8_t key_pw1_read(void)
{
  return (uint8_t)(HAL_GPIO_ReadPin(KEY_PW1_GPIO_Port, KEY_PW1_Pin) == GPIO_PIN_RESET);
}

uint8_t key_pw2_read(void)
{
  return (uint8_t)(HAL_GPIO_ReadPin(KEY_PW2_GPIO_Port, KEY_PW2_Pin) == GPIO_PIN_RESET);
}

void update_leds(LockState state)
{
  led_locked_off();
  led_unlocked_off();
  led_alarm_off();

  switch (state) {
    case STATE_LOCKED:
      led_locked_on();
      break;
    case STATE_UNLOCKED:
    case STATE_OPEN:
      led_unlocked_on();
      break;
    case STATE_ALARM:
      led_alarm_on();
      break;
    default:
      break;
  }
}

/* USER CODE END 2 */

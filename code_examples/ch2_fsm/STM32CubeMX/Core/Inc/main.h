/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY_PW0_Pin GPIO_PIN_0
#define KEY_PW0_GPIO_Port GPIOA
#define KEY_PW1_Pin GPIO_PIN_1
#define KEY_PW1_GPIO_Port GPIOA
#define KEY_PW2_Pin GPIO_PIN_2
#define KEY_PW2_GPIO_Port GPIOA
#define LED_LOCKED_Pin GPIO_PIN_0
#define LED_LOCKED_GPIO_Port GPIOB
#define LED_UNLOCKED_Pin GPIO_PIN_1
#define LED_UNLOCKED_GPIO_Port GPIOB
#define LED_ALARM_Pin GPIO_PIN_2
#define LED_ALARM_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_3
#define BUZZER_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

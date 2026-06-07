/**
  ******************************************************************************
  * @file     stm32f4xx_hal.h
  * @brief    Header file of HAL Library.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_HAL_H
#define __STM32F4xx_HAL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED

/* ########################### System Configuration ######################### */
#define  HSI_VALUE    ((uint32_t)16000000)    /*!< Value of the Internal oscillator in Hz */
#define  HSE_VALUE    ((uint32_t)8000000)     /*!< Value of the External oscillator in Hz */
#define  LSE_VALUE    ((uint32_t)32768)       /*!< Value of the External Low oscillator in Hz */
#define  LSI_VALUE    ((uint32_t)32000)       /*!< Value of the Internal Low oscillator in Hz */

/* Exported macro ------------------------------------------------------------*/
#define __HAL_RCC_GPIOA_CLK_ENABLE()           do { __IO uint32_t tmpreg; \
                                                   SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN); \
                                                   tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN); \
                                                   UNUSED(tmpreg); \
                                                 } while(0)

#define __HAL_RCC_GPIOB_CLK_ENABLE()           do { __IO uint32_t tmpreg; \
                                                   SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN); \
                                                   tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOBEN); \
                                                   UNUSED(tmpreg); \
                                                 } while(0)

/* Exported functions ------------------------------------------------------- */
void HAL_Init(void);
void HAL_Delay(uint32_t Delay);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
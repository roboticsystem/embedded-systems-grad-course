/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f1xx_hal_msp.c
  * @brief        MSP Initialization and de-Initialization codes.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

void HAL_MspInit(void)
{
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /** NOJTAG: release PB3/PB4/PA15 for GPIO (BUZZER on PB3) */
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

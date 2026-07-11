/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   本文件包含应用共用的宏定义（CubeMX 生成风格）。
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* ---------- 引脚宏定义（对应 .ioc 中 GPIO_Label）---------- */
#define LED_ONBOARD_Pin       GPIO_PIN_13   /* 板载 LED，低电平点亮 */
#define LED_ONBOARD_GPIO_Port GPIOC
#define LED_EXT_Pin           GPIO_PIN_0    /* 扩展 LED */
#define LED_EXT_GPIO_Port     GPIOA

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

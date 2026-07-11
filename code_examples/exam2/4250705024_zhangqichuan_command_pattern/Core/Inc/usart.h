/**
 * @file    usart.h
 * @brief   USART1 初始化头文件（CubeMX 生成风格）
 * @note    PA9 = TX, PA10 = RX，115200 8N1，中断接收
 */

#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "main.h"                    /* 提供 Error_Handler 原型与引脚宏 */

extern UART_HandleTypeDef huart1;   /* USART1 句柄，供 command.c 回显调用 */

void MX_USART1_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

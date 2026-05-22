#ifndef USART_H
#define USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* USART_H */

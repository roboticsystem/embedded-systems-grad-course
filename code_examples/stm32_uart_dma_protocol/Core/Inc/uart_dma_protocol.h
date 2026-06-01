#ifndef __UART_DMA_PROTOCOL_H
#define __UART_DMA_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f1xx_hal.h"

#define UART_DMA_RX_BUFFER_SIZE 128U
#define UART_DMA_TX_BUFFER_SIZE 96U

extern uint8_t uart_dma_rx_buffer[UART_DMA_RX_BUFFER_SIZE];

void UART_DMA_Protocol_Start(void);
HAL_StatusTypeDef UART_DMA_Protocol_Send(const uint8_t *data, uint16_t length);
uint8_t UART_DMA_Protocol_IsTxBusy(void);

#ifdef __cplusplus
}
#endif

#endif

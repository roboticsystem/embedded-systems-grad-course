#include "uart_dma_protocol.h"
#include "app_protocol.h"
#include "usart.h"
#include <string.h>

uint8_t uart_dma_rx_buffer[UART_DMA_RX_BUFFER_SIZE];

static uint8_t uart_dma_tx_buffer[UART_DMA_TX_BUFFER_SIZE];
static volatile uint8_t uart_dma_tx_busy;

void UART_DMA_Protocol_Start(void)
{
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_dma_rx_buffer,
                                     UART_DMA_RX_BUFFER_SIZE) != HAL_OK) {
        Error_Handler();
    }

    if (huart1.hdmarx != NULL) {
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
    }
}

HAL_StatusTypeDef UART_DMA_Protocol_Send(const uint8_t *data, uint16_t length)
{
    if ((data == NULL) || (length == 0U) ||
        (length > UART_DMA_TX_BUFFER_SIZE)) {
        return HAL_ERROR;
    }

    if (uart_dma_tx_busy != 0U) {
        return HAL_BUSY;
    }

    memcpy(uart_dma_tx_buffer, data, length);
    uart_dma_tx_busy = 1U;

    if (HAL_UART_Transmit_DMA(&huart1, uart_dma_tx_buffer, length) != HAL_OK) {
        uart_dma_tx_busy = 0U;
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint8_t UART_DMA_Protocol_IsTxBusy(void)
{
    return uart_dma_tx_busy;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart->Instance == USART1) {
        Protocol_ParseBytes(uart_dma_rx_buffer, size);
        UART_DMA_Protocol_Start();
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_dma_tx_busy = 0U;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_dma_tx_busy = 0U;
        UART_DMA_Protocol_Start();
    }
}

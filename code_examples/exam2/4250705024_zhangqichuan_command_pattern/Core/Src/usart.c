/**
 * @file    usart.c
 * @brief   USART1 初始化 + 接收完成回调（CubeMX 生成风格）
 * @note    PA9 = TX, PA10 = RX，115200 8N1，中断接收单字节
 */

#include "usart.h"
#include "command.h"

UART_HandleTypeDef huart1;          /* USART1 句柄（全局唯一定义） */
static uint8_t rx_byte;             /* 中断单字节接收缓冲（本文件唯一定义） */

/**
 * @brief  USART1 初始化：115200 8N1，无硬件流控
 */
void MX_USART1_UART_Init(void)
{
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    /* 启动首次中断接收，之后每次回调里重新武装 */
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/**
 * @brief  UART 接收完成回调：将收到的字节喂入命令环形缓冲
 * @note   运行在中断上下文，仅做入队 + 重新挂起接收，不做耗时操作
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        cmd_rx_feed(rx_byte);                        /* 入环形缓冲 */
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);    /* 重新武装中断 */
    }
}

/**
 * @brief  UART 错误回调：帧/校验/过载错误后重新武装接收，避免接收永久停摆
 * @note   线路噪声触发 FE/PE/ORE 时 HAL 会终止接收，此处重新挂起中断接收以恢复
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);    /* 出错后重新武装，保证鲁棒性 */
    }
}

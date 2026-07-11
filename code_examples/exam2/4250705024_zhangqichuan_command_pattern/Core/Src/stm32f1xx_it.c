/**
 * @file    stm32f1xx_it.c
 * @brief   Interrupt Service Routines.
 */

#include "main.h"
#include "stm32f1xx_it.h"
#include "usart.h"

void NMI_Handler(void)
{
    while (1) {}
}

void HardFault_Handler(void)
{
    while (1) {}
}

void MemManage_Handler(void)
{
    while (1) {}
}

void BusFault_Handler(void)
{
    while (1) {}
}

void UsageFault_Handler(void)
{
    while (1) {}
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

/**
 * @brief  USART1 全局中断服务函数
 * @note   转交 HAL 库统一处理，内部触发 HAL_UART_RxCpltCallback
 */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

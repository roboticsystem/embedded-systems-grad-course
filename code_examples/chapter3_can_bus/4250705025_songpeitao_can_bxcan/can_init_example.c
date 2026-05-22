#include "main.h"

CAN_HandleTypeDef hcan;

/*
 * CubeMX reference settings:
 * - MCU: STM32F103C8T6
 * - CAN peripheral clock: PCLK1 = 36 MHz
 * - Prescaler = 4
 * - Sync_Seg = 1 TQ, BS1 = 6 TQ, BS2 = 2 TQ
 *
 * CAN bit rate = 36 MHz / 4 / (1 + 6 + 2) = 1 Mbps
 * Sample point = (1 + 6) / (1 + 6 + 2) = 77.8%
 *
 * If CubeMX has already generated MX_CAN_Init() in can.c, copy only the
 * parameter values below into the generated function instead of compiling
 * this file at the same time.
 */
void MX_CAN_Init(void)
{
    hcan.Instance = CAN1;
    hcan.Init.Prescaler = 4;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan.Init.TimeTriggeredMode = DISABLE;
    hcan.Init.AutoBusOff = ENABLE;
    hcan.Init.AutoWakeUp = DISABLE;
    hcan.Init.AutoRetransmission = ENABLE;
    hcan.Init.ReceiveFifoLocked = DISABLE;
    hcan.Init.TransmitFifoPriority = DISABLE;

    if (HAL_CAN_Init(&hcan) != HAL_OK) {
        Error_Handler();
    }
}

#include "main.h"

CAN_HandleTypeDef hcan;

void MX_CAN_Init(void)
{
    hcan.Instance = CAN1;

#if CAN_SIMULATION_LOOPBACK
    /*
     * PicSimLab loopback reference:
     * PCLK1 = 32 MHz, Prescaler = 4, BS1 = 6 TQ, BS2 = 1 TQ.
     * CAN bit rate = 32 MHz / 4 / (1 + 6 + 1) = 1 Mbps.
     * Sample point = (1 + 6) / (1 + 6 + 1) = 87.5%.
     */
    hcan.Init.Prescaler = 4;
    hcan.Init.Mode = CAN_MODE_LOOPBACK;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
#else
    /*
     * CubeMX reference:
     * PCLK1 = 36 MHz, Prescaler = 4, BS1 = 6 TQ, BS2 = 2 TQ.
     * CAN bit rate = 36 MHz / 4 / (1 + 6 + 2) = 1 Mbps.
     * Sample point = (1 + 6) / (1 + 6 + 2) = 77.8%.
     */
    hcan.Init.Prescaler = 4;
    hcan.Init.Mode = CAN_MODE_NORMAL;
    hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
    hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
#endif
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

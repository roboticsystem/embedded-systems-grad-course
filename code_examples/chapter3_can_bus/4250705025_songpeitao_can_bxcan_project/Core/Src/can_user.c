#include "main.h"
#include "can_user.h"

#define CAN_MOTOR_COUNT            4U
#define CAN_MOTOR_CMD_BASE_ID      0x200U
#define CAN_MOTOR_FB_BASE_ID       0x210U
#define CAN_MOTOR_FB_FIRST_ID      0x211U
#define CAN_MOTOR_FB_LAST_ID       0x214U

#define CAN_FILTER_BANK_BRINGUP    0U
#define CAN_FILTER_BANK_MOTOR_MASK 1U
#define CAN_FILTER_BANK_MOTOR_LIST 2U
#define CAN_SLAVE_START_BANK       14U

volatile CAN_MotorFeedback_t g_can_motor_feedback[CAN_MOTOR_COUNT];

static uint16_t CAN_StdIdToFilter16(uint16_t std_id)
{
    return (uint16_t)(std_id << 5);
}

void CAN_FilterAcceptAll(void)
{
    CAN_FilterTypeDef filter;

    filter.FilterBank = CAN_FILTER_BANK_BRINGUP;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = 0x0000;
    filter.FilterIdLow = 0x0000;
    filter.FilterMaskIdHigh = 0x0000;
    filter.FilterMaskIdLow = 0x0000;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = CAN_SLAVE_START_BANK;

    if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK) {
        Error_Handler();
    }
}

void CAN_ConfigMotorRangeFilter(void)
{
    CAN_FilterTypeDef filter;

    /*
     * Accept 0x210-0x21F standard data frames:
     * (StdId & 0x7F0) == 0x210, IDE == 0, RTR == 0.
     */
    filter.FilterBank = CAN_FILTER_BANK_MOTOR_MASK;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterIdHigh = (uint16_t)(CAN_MOTOR_FB_BASE_ID << 5);
    filter.FilterIdLow = 0x0000;
    filter.FilterMaskIdHigh = (uint16_t)(0x7F0U << 5);
    filter.FilterMaskIdLow = 0x0006;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = CAN_SLAVE_START_BANK;

    if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK) {
        Error_Handler();
    }
}

void CAN_ConfigMotorListFilter(void)
{
    CAN_FilterTypeDef filter;

    /*
     * Accept exactly 0x211, 0x212, 0x213 and 0x214.
     * In 16-bit filter scale, each standard ID is shifted left by 5.
     */
    filter.FilterBank = CAN_FILTER_BANK_MOTOR_LIST;
    filter.FilterMode = CAN_FILTERMODE_IDLIST;
    filter.FilterScale = CAN_FILTERSCALE_16BIT;
    filter.FilterIdHigh = CAN_StdIdToFilter16(0x211U);
    filter.FilterIdLow = CAN_StdIdToFilter16(0x212U);
    filter.FilterMaskIdHigh = CAN_StdIdToFilter16(0x213U);
    filter.FilterMaskIdLow = CAN_StdIdToFilter16(0x214U);
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = CAN_SLAVE_START_BANK;

    if (HAL_CAN_ConfigFilter(&hcan, &filter) != HAL_OK) {
        Error_Handler();
    }
}

void CAN_UserStart(void)
{
    CAN_ConfigMotorRangeFilter();

    if (HAL_CAN_Start(&hcan) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_CAN_ActivateNotification(&hcan,
            CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
        Error_Handler();
    }
}

HAL_StatusTypeDef CAN_SendMotorCommand(uint8_t motor_id,
                                       int16_t target_rpm,
                                       uint8_t enable)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[4];
    uint32_t tx_mailbox;

    if (motor_id == 0U || motor_id > CAN_MOTOR_COUNT) {
        return HAL_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0U) {
        return HAL_BUSY;
    }

    tx_header.StdId = CAN_MOTOR_CMD_BASE_ID + motor_id;
    tx_header.ExtId = 0U;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 4U;
    tx_header.TransmitGlobalTime = DISABLE;

    tx_data[0] = (uint8_t)((uint16_t)target_rpm >> 8);
    tx_data[1] = (uint8_t)((uint16_t)target_rpm);
    tx_data[2] = enable ? 1U : 0U;
    tx_data[3] = 0U;

    return HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
}

HAL_StatusTypeDef CAN_SendSimulatedMotorFeedback(uint8_t motor_id,
                                                 int16_t rpm,
                                                 int16_t current_ma,
                                                 uint16_t position)
{
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[8];
    uint32_t tx_mailbox;

    if (motor_id == 0U || motor_id > CAN_MOTOR_COUNT) {
        return HAL_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0U) {
        return HAL_BUSY;
    }

    tx_header.StdId = CAN_MOTOR_FB_BASE_ID + motor_id;
    tx_header.ExtId = 0U;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = 6U;
    tx_header.TransmitGlobalTime = DISABLE;

    tx_data[0] = (uint8_t)((uint16_t)rpm >> 8);
    tx_data[1] = (uint8_t)((uint16_t)rpm);
    tx_data[2] = (uint8_t)((uint16_t)current_ma >> 8);
    tx_data[3] = (uint8_t)((uint16_t)current_ma);
    tx_data[4] = (uint8_t)(position >> 8);
    tx_data[5] = (uint8_t)position;
    tx_data[6] = 0U;
    tx_data[7] = 0U;

    return HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data, &tx_mailbox);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *can)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];
    uint8_t index;

    if (can->Instance != CAN1) {
        return;
    }

    if (HAL_CAN_GetRxMessage(can, CAN_RX_FIFO0,
                             &rx_header, rx_data) != HAL_OK) {
        return;
    }

    if (rx_header.IDE != CAN_ID_STD ||
        rx_header.RTR != CAN_RTR_DATA ||
        rx_header.DLC < 6U ||
        rx_header.StdId < CAN_MOTOR_FB_FIRST_ID ||
        rx_header.StdId > CAN_MOTOR_FB_LAST_ID) {
        return;
    }

    index = (uint8_t)(rx_header.StdId - CAN_MOTOR_FB_FIRST_ID);
    g_can_motor_feedback[index].rpm =
        (int16_t)(((uint16_t)rx_data[0] << 8) | rx_data[1]);
    g_can_motor_feedback[index].current_ma =
        (int16_t)(((uint16_t)rx_data[2] << 8) | rx_data[3]);
    g_can_motor_feedback[index].position =
        (uint16_t)(((uint16_t)rx_data[4] << 8) | rx_data[5]);
    g_can_motor_feedback[index].last_tick = HAL_GetTick();
}

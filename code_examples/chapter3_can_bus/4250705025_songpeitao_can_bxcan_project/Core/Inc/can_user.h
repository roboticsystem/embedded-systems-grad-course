#ifndef CAN_USER_H
#define CAN_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

typedef struct {
    int16_t rpm;
    int16_t current_ma;
    uint16_t position;
    uint32_t last_tick;
} CAN_MotorFeedback_t;

extern CAN_HandleTypeDef hcan;
extern volatile CAN_MotorFeedback_t g_can_motor_feedback[4];

void CAN_UserStart(void);
void CAN_FilterAcceptAll(void);
void CAN_ConfigMotorRangeFilter(void);
void CAN_ConfigMotorListFilter(void);

HAL_StatusTypeDef CAN_SendMotorCommand(uint8_t motor_id,
                                       int16_t target_rpm,
                                       uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif /* CAN_USER_H */

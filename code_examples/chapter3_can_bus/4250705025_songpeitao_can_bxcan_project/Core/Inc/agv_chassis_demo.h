#ifndef AGV_CHASSIS_DEMO_H
#define AGV_CHASSIS_DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void AGV_CAN_SetTargetRpm(uint8_t motor_id, int16_t rpm);
void AGV_CAN_10msTask(void);
void AGV_CAN_StopAllMotors(void);
uint8_t AGV_CAN_IsMotorFeedbackFresh(uint8_t motor_id, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* AGV_CHASSIS_DEMO_H */

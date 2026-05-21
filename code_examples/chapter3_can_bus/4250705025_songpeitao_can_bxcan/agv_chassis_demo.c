#include "can_user.h"
#include "agv_chassis_demo.h"

#define AGV_MOTOR_COUNT 4U

static int16_t s_target_rpm[AGV_MOTOR_COUNT];

void AGV_CAN_SetTargetRpm(uint8_t motor_id, int16_t rpm)
{
    if (motor_id == 0U || motor_id > AGV_MOTOR_COUNT) {
        return;
    }

    s_target_rpm[motor_id - 1U] = rpm;
}

void AGV_CAN_10msTask(void)
{
    uint8_t motor_id;

    for (motor_id = 1U; motor_id <= AGV_MOTOR_COUNT; motor_id++) {
        (void)CAN_SendMotorCommand(motor_id,
                                   s_target_rpm[motor_id - 1U],
                                   1U);
    }
}

void AGV_CAN_StopAllMotors(void)
{
    uint8_t motor_id;

    for (motor_id = 1U; motor_id <= AGV_MOTOR_COUNT; motor_id++) {
        s_target_rpm[motor_id - 1U] = 0;
        (void)CAN_SendMotorCommand(motor_id, 0, 0U);
    }
}

uint8_t AGV_CAN_IsMotorFeedbackFresh(uint8_t motor_id, uint32_t timeout_ms)
{
    uint32_t now;
    uint32_t last_tick;

    if (motor_id == 0U || motor_id > AGV_MOTOR_COUNT) {
        return 0U;
    }

    now = HAL_GetTick();
    last_tick = g_can_motor_feedback[motor_id - 1U].last_tick;

    return ((now - last_tick) <= timeout_ms) ? 1U : 0U;
}

/**
  ******************************************************************************
  * @file           : fsm.h
  * @brief          : 有限状态机（FSM）接口定义
  ******************************************************************************
  */

#ifndef __FSM_H
#define __FSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    STATE_LOCKED,
    STATE_UNLOCKED,
    STATE_OPEN,
    STATE_ALARM,
    STATE_MAX
} LockState;

typedef enum {
    EVENT_PASSWORD_OK,
    EVENT_PASSWORD_ERR,
    EVENT_ERR_LIMIT,
    EVENT_TIMEOUT,
    EVENT_DOOR_PUSH,
    EVENT_DOOR_CLOSE,
    EVENT_ADMIN_RESET,
    EVENT_MAX
} LockEvent;

#define FSM_IMPLEMENTATION_SWITCH_CASE  0
#define FSM_IMPLEMENTATION_TABLE_DRIVEN 1

void fsm_init(void);
void fsm_process(LockEvent event);
LockState fsm_get_state(void);
uint8_t fsm_get_error_count(void);

#ifdef __cplusplus
}
#endif

#endif /* __FSM_H */

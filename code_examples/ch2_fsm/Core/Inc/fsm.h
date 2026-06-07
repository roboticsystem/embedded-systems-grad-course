/**
  ******************************************************************************
  * @file           : fsm.h
  * @brief          : 有限状态机（FSM）接口定义
  * @author         : Zhou Qing
  * @date           : 2024
  ******************************************************************************
  */

#ifndef __FSM_H
#define __FSM_H

#ifdef __cplusplus
extern "C" {
#endif

/* 状态定义 */
typedef enum {
    STATE_LOCKED,    /* 锁定状态 */
    STATE_UNLOCKED,  /* 解锁状态 */
    STATE_OPEN,      /* 门打开状态 */
    STATE_ALARM,     /* 报警状态 */
    STATE_MAX        /* 状态数量 */
} LockState;

/* 事件定义 */
typedef enum {
    EVENT_PASSWORD_OK,   /* 密码正确 */
    EVENT_PASSWORD_ERR,  /* 密码错误 */
    EVENT_ERR_LIMIT,     /* 错误次数达到上限 */
    EVENT_TIMEOUT,       /* 超时事件 */
    EVENT_DOOR_PUSH,     /* 推门事件 */
    EVENT_DOOR_CLOSE,    /* 门关闭事件 */
    EVENT_ADMIN_RESET,   /* 管理员复位 */
    EVENT_MAX            /* 事件数量 */
} LockEvent;

/* 实现方式选择 */
#define FSM_IMPLEMENTATION_SWITCH_CASE  0  /* Switch-Case实现 */
#define FSM_IMPLEMENTATION_TABLE_DRIVEN 1  /* 表驱动实现 */

/* 函数声明 */
void fsm_init(void);
void fsm_process(LockEvent event);
LockState fsm_get_state(void);
uint8_t fsm_get_error_count(void);

#ifdef __cplusplus
}
#endif

#endif /* __FSM_H */
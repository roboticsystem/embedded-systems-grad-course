/**
  ******************************************************************************
  * @file           : fsm.c
  * @brief          : 有限状态机（FSM）实现
  *                   包含Switch-Case和表驱动两种实现方式
  * @author         : Zhou Qing
  * @date           : 2024
  ******************************************************************************
  */

#include "fsm.h"
#include "gpio.h"

/* 静态变量 */
static LockState s_state = STATE_LOCKED;  /* 当前状态 */
static uint8_t   s_err_count = 0;         /* 错误计数 */

/* 动作函数声明 */
static void lock_open(void);
static void lock_close(void);
static void alarm_trigger(void);
static void alarm_stop(void);
static void count_error(void);
static void log_entry(void);

/**
  * @brief  初始化状态机
  * @param  无
  * @retval 无
  */
void fsm_init(void) {
    s_state = STATE_LOCKED;
    s_err_count = 0;
    update_leds(s_state);
}

/**
  * @brief  获取当前状态
  * @param  无
  * @retval 当前状态
  */
LockState fsm_get_state(void) {
    return s_state;
}

/**
  * @brief  获取错误计数
  * @param  无
  * @retval 错误计数值
  */
uint8_t fsm_get_error_count(void) {
    return s_err_count;
}

#if FSM_IMPLEMENTATION_SWITCH_CASE

/**
  * @brief  Switch-Case方式处理状态机事件
  * @param  event: 触发的事件
  * @retval 无
  */
void fsm_process(LockEvent event) {
    switch (s_state) {
        case STATE_LOCKED:
            if (event == EVENT_PASSWORD_OK) {
                lock_open();           /* 动作：打开门锁 */
                s_err_count = 0;
                s_state = STATE_UNLOCKED;
            } else if (event == EVENT_PASSWORD_ERR) {
                count_error();         /* 动作：计数错误 */
                if (s_err_count >= 3) {
                    alarm_trigger();   /* 动作：触发报警 */
                    s_state = STATE_ALARM;
                }
            }
            break;

        case STATE_UNLOCKED:
            if (event == EVENT_TIMEOUT) {
                lock_close();          /* 动作：关闭门锁 */
                s_state = STATE_LOCKED;
            } else if (event == EVENT_DOOR_PUSH) {
                log_entry();           /* 动作：记录开门 */
                s_state = STATE_OPEN;
            }
            break;

        case STATE_OPEN:
            if (event == EVENT_DOOR_CLOSE) {
                s_state = STATE_UNLOCKED;
            }
            break;

        case STATE_ALARM:
            if (event == EVENT_ADMIN_RESET) {
                alarm_stop();          /* 动作：停止报警 */
                s_err_count = 0;
                s_state = STATE_LOCKED;
            }
            break;

        default:
            /* 未知状态，重置 */
            s_state = STATE_LOCKED;
            s_err_count = 0;
            break;
    }
    
    update_leds(s_state);
}

#elif FSM_IMPLEMENTATION_TABLE_DRIVEN

/* 动作函数指针类型 */
typedef void (*ActionFn)(void);

/* 状态转移结构体 */
typedef struct {
    LockState   next_state;   /* 转移后的状态 */
    ActionFn    action;       /* 执行的动作 */
} Transition;

/* 状态转移表 [当前状态][事件] */
static const Transition fsm_table[STATE_MAX][EVENT_MAX] = {
    /* STATE_LOCKED */
    [STATE_LOCKED] = {
        [EVENT_PASSWORD_OK]  = { STATE_UNLOCKED, lock_open     },
        [EVENT_PASSWORD_ERR] = { STATE_LOCKED,   count_error   },
        [EVENT_ERR_LIMIT]    = { STATE_ALARM,     alarm_trigger },
    },
    /* STATE_UNLOCKED */
    [STATE_UNLOCKED] = {
        [EVENT_TIMEOUT]   = { STATE_LOCKED,  lock_close },
        [EVENT_DOOR_PUSH] = { STATE_OPEN,    log_entry  },
    },
    /* STATE_OPEN */
    [STATE_OPEN] = {
        [EVENT_DOOR_CLOSE] = { STATE_UNLOCKED, NULL },
    },
    /* STATE_ALARM */
    [STATE_ALARM] = {
        [EVENT_ADMIN_RESET] = { STATE_LOCKED, alarm_stop },
    },
};

/**
  * @brief  表驱动方式处理状态机事件
  * @param  event: 触发的事件
  * @retval 无
  */
void fsm_process(LockEvent event) {
    if (event >= EVENT_MAX || s_state >= STATE_MAX) {
        return;  /* 无效事件或状态 */
    }
    
    const Transition *t = &fsm_table[s_state][event];
    
    if (t->action != NULL) {
        t->action();
    }
    
    s_state = t->next_state;
    update_leds(s_state);
}

#endif /* FSM_IMPLEMENTATION */

/**
  * @brief  打开门锁动作
  * @param  无
  * @retval 无
  */
static void lock_open(void) {
    /* 实际应用中这里会控制门锁电机 */
}

/**
  * @brief  关闭门锁动作
  * @param  无
  * @retval 无
  */
static void lock_close(void) {
    /* 实际应用中这里会控制门锁电机 */
}

/**
  * @brief  触发报警动作
  * @param  无
  * @retval 无
  */
static void alarm_trigger(void) {
    buzzer_on();
}

/**
  * @brief  停止报警动作
  * @param  无
  * @retval 无
  */
static void alarm_stop(void) {
    buzzer_off();
}

/**
  * @brief  计数错误动作
  * @param  无
  * @retval 无
  */
static void count_error(void) {
    s_err_count++;
}

/**
  * @brief  记录开门时间动作
  * @param  无
  * @retval 无
  */
static void log_entry(void) {
    /* 实际应用中这里会记录开门时间戳 */
}
/**
  ******************************************************************************
  * @file           : fsm.c
  * @brief          : 有限状态机（FSM）实现（Switch-Case / 表驱动）
  ******************************************************************************
  */

#include "fsm.h"
#include "gpio.h"

static LockState s_state = STATE_LOCKED;
static uint8_t   s_err_count = 0;

static void lock_open(void);
static void lock_close(void);
static void alarm_trigger(void);
static void alarm_stop(void);
static void count_error(void);
static void log_entry(void);

void fsm_init(void)
{
    s_state = STATE_LOCKED;
    s_err_count = 0;
    update_leds(s_state);
}

LockState fsm_get_state(void)
{
    return s_state;
}

uint8_t fsm_get_error_count(void)
{
    return s_err_count;
}

#if FSM_IMPLEMENTATION_SWITCH_CASE

void fsm_process(LockEvent event)
{
    switch (s_state) {
        case STATE_LOCKED:
            if (event == EVENT_PASSWORD_OK) {
                lock_open();
                s_err_count = 0;
                s_state = STATE_UNLOCKED;
            } else if (event == EVENT_PASSWORD_ERR) {
                count_error();
                if (s_err_count >= 3) {
                    alarm_trigger();
                    s_state = STATE_ALARM;
                }
            }
            break;

        case STATE_UNLOCKED:
            if (event == EVENT_TIMEOUT) {
                lock_close();
                s_state = STATE_LOCKED;
            } else if (event == EVENT_DOOR_PUSH) {
                log_entry();
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
                alarm_stop();
                s_err_count = 0;
                s_state = STATE_LOCKED;
            }
            break;

        default:
            s_state = STATE_LOCKED;
            s_err_count = 0;
            break;
    }

    update_leds(s_state);
}

#elif FSM_IMPLEMENTATION_TABLE_DRIVEN

typedef void (*ActionFn)(void);

typedef struct {
    LockState next_state;
    ActionFn  action;
} Transition;

static const Transition fsm_table[STATE_MAX][EVENT_MAX] = {
    [STATE_LOCKED] = {
        [EVENT_PASSWORD_OK]  = { STATE_UNLOCKED, lock_open     },
        [EVENT_PASSWORD_ERR] = { STATE_LOCKED,   count_error   },
        [EVENT_ERR_LIMIT]    = { STATE_ALARM,    alarm_trigger },
    },
    [STATE_UNLOCKED] = {
        [EVENT_TIMEOUT]   = { STATE_LOCKED, lock_close },
        [EVENT_DOOR_PUSH] = { STATE_OPEN,   log_entry  },
    },
    [STATE_OPEN] = {
        [EVENT_DOOR_CLOSE] = { STATE_UNLOCKED, NULL },
    },
    [STATE_ALARM] = {
        [EVENT_ADMIN_RESET] = { STATE_LOCKED, alarm_stop },
    },
};

void fsm_process(LockEvent event)
{
    if (event >= EVENT_MAX || s_state >= STATE_MAX) {
        return;
    }

    const Transition *t = &fsm_table[s_state][event];

    if (t->action != NULL) {
        t->action();
    }

    s_state = t->next_state;
    update_leds(s_state);
}

#endif

static void lock_open(void) {}
static void lock_close(void) {}

static void alarm_trigger(void)
{
    buzzer_on();
}

static void alarm_stop(void)
{
    buzzer_off();
}

static void count_error(void)
{
    s_err_count++;
}

static void log_entry(void) {}

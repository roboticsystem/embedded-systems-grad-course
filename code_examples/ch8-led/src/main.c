/**
 * @file    main.c
 * @brief   第 8.2 节 LED 实验主程序
 *
 * 通过宏选择要运行的实验：
 *   - #define DEMO_SINGLE_LED    1  实验 1：PC13 板载 LED 闪烁
 *   - #define DEMO_WATERFALL     1  实验 2：PA0~PA7 八路流水灯
 *   - #define DEMO_BREATHE       1  实验 3：PA0 PWM 呼吸灯
 *
 * 编译：make
 * 烧录：make flash
 */
#include "led.h"

/* ========== 选择要运行的实验 ========== */
#define DEMO_SINGLE_LED   0   /* 设 1 运行单 LED 闪烁 */
#define DEMO_WATERFALL    0   /* 设 1 运行八路流水灯 */
#define DEMO_BREATHE      1   /* 设 1 运行 PWM 呼吸灯 */

/* ========== 简单延时（主循环用） ========== */
static void delay_loops(volatile uint32_t n)
{
    for (volatile uint32_t i = 0; i < n; i++) { /* nop */ }
}

int main(void)
{
#if DEMO_SINGLE_LED
    /* 实验 1：板载 LED 闪烁 5 次后结束 */
    LED_SingleDemo();
    while (1) { /* 空循环，可插入低功耗 */ }

#elif DEMO_WATERFALL
    /* 实验 2：八路流水灯，无限循环 */
    LED_Waterfall();

#elif DEMO_BREATHE
    /* 实验 3：呼吸灯，无限循环 */
    LED_PWM_Init();
    while (1) {
        LED_Breathe();
    }
#endif

    return 0;
}

/* ========== 弱符号：HAL_Delay 替代实现 ========== */
void HAL_Delay(volatile uint32_t Delay)
{
    for (volatile uint32_t i = 0; i < Delay * 4000; i++) { /* nop */ }
}
*** End of File

#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>

/* ---- 实验 1：单 LED 控制（PC13 板载 LED） ---- */
/** PC13 LED 低电平点亮，高电平熄灭 */
#define LED_ON()   *((volatile uint32_t *)(0x40011000 + 0x0C)) &= ~(1 << 13)
#define LED_OFF()  *((volatile uint32_t *)(0x40011000 + 0x0C)) |=  (1 << 13)
#define LED_TOGG() *((volatile uint32_t *)(0x40011000 + 0x0C)) ^=  (1 << 13)

void LED_SingleDemo(void);    /* PC13 闪烁演示 */

/* ---- 实验 2：八路流水灯（PA0~PA7） ---- */
void LED_Waterfall(void);     /* 流水灯效果 */

/* ---- 实验 3：PWM 呼吸灯（PA0 / TIM2_CH1） ---- */
void LED_PWM_Init(void);      /* 初始化 TIM2_CH1 PWM */
void LED_Breathe(void);       /* 呼吸灯效果 */

#endif
*** End of File

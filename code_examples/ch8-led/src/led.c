/**
 * @file    led.c
 * @brief   第 8.2 节 LED 实验驱动源码
 *          直接操作 STM32F103 寄存器，无 HAL 依赖。
 *          三个实验可独立运行，通过 main.c 中宏选择。
 */
#include "led.h"

/* ========== 寄存器基地址 ========== */
#define RCC_BASE    0x40021000
#define GPIOA_BASE  0x40010800
#define GPIOC_BASE  0x40011000
#define TIM2_BASE   0x40000000

#define RCC_APB2ENR (*((volatile uint32_t *)(RCC_BASE   + 0x18)))
#define RCC_APB1ENR (*((volatile uint32_t *)(RCC_BASE   + 0x1C)))

#define GPIOA_CRL   (*((volatile uint32_t *)(GPIOA_BASE + 0x00)))
#define GPIOA_ODR   (*((volatile uint32_t *)(GPIOA_BASE + 0x0C)))
#define GPIOC_CRH   (*((volatile uint32_t *)(GPIOC_BASE + 0x04)))
#define GPIOC_ODR   (*((volatile uint32_t *)(GPIOC_BASE + 0x0C)))

#define TIM2_CR1    (*((volatile uint32_t *)(TIM2_BASE  + 0x00)))
#define TIM2_PSC    (*((volatile uint32_t *)(TIM2_BASE  + 0x28)))
#define TIM2_ARR    (*((volatile uint32_t *)(TIM2_BASE  + 0x2C)))
#define TIM2_CCR1   (*((volatile uint32_t *)(TIM2_BASE  + 0x34)))
#define TIM2_CCMR1  (*((volatile uint32_t *)(TIM2_BASE  + 0x18)))
#define TIM2_CCER   (*((volatile uint32_t *)(TIM2_BASE  + 0x20)))
#define TIM2_SR     (*((volatile uint32_t *)(TIM2_BASE  + 0x10)))

/* ========== 简单延时（仅供参考，非精确） ========== */
static void delay_ms(volatile uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 4000; i++) { /* nop */ }
}

static void delay_us(volatile uint32_t us)
{
    for (volatile uint32_t i = 0; i < us * 4; i++) { /* nop */ }
}

/* ========== 初始化函数 ========== */

/** 初始化 PC13 为推挽输出（板载 LED） */
void LED_SingleInit(void)
{
    RCC_APB2ENR |= (1 << 4);         /* 使能 GPIOC 时钟 */
    GPIOC_CRH   = (GPIOC_CRH & ~0xF0000000) | 0x20000000; /* PC13: PP out 50MHz */
    LED_OFF();                       /* 初始熄灭 */
}

/** 初始化 PA0~PA7 为推挽输出（流水灯） */
void LED_WaterfallInit(void)
{
    RCC_APB2ENR |= (1 << 2);         /* 使能 GPIOA 时钟 */
    GPIOA_CRL   = 0x22222222;        /* PA0~PA7: PP out 50MHz */
    GPIOA_ODR  |= 0x00FF;            /* 初始全部熄灭（高电平） */
}

/* ========== 实验 1：单 LED 控制 ========== */

void LED_SingleDemo(void)
{
    LED_SingleInit();

    for (int i = 0; i < 10; i++) {
        LED_ON();
        delay_ms(300);
        LED_OFF();
        delay_ms(300);
    }
}

/* ========== 实验 2：八路流水灯 ========== */

void LED_Waterfall(void)
{
    LED_WaterfallInit();

    while (1) {
        for (int i = 0; i < 8; i++) {
            GPIOA_ODR = (GPIOA_ODR | 0x00FF);  /* 熄灭所有 */
            GPIOA_ODR = (GPIOA_ODR & ~(1 << i)); /* 点亮第 i 个 LED */
            delay_ms(200);
        }
    }
}

/* ========== 实验 3：PWM 呼吸灯 ========== */

void LED_PWM_Init(void)
{
    /* 使能 GPIOA 时钟，配置 PA0 为复用推挽输出 */
    RCC_APB2ENR |= (1 << 2);         /* GPIOA 时钟 */
    GPIOA_CRL   = (GPIOA_CRL & ~0x0F) | 0x0B;  /* PA0: AF PP out 50MHz (CNF=10, MODE=11) */

    /* 使能 TIM2 时钟 */
    RCC_APB1ENR |= (1 << 0);

    /* TIM2 配置：72MHz / (72-1) = 1MHz，ARR=1000，PWM 频率 = 1MHz/1000 = 1kHz */
    TIM2_PSC = 72 - 1;               /* 预分频：72MHz / 72 = 1MHz */
    TIM2_ARR = 1000 - 1;             /* 自动重载：1000 个计数 */

    /* 配置 CH1 为 PWM1 模式预装载使能 */
    TIM2_CCMR1 = (TIM2_CCMR1 & ~0xFF) | 0x68;  /* OC1M=110 (PWM1), OC1PE=1 */

    TIM2_CCER |= (1 << 0);           /* 使能 CH1 输出 */
    TIM2_CR1  |= (1 << 0);           /* 使能 TIM2 */
}

void LED_Breathe(void)
{
    /* 占空比 0 → 100% → 0 循环 */
    for (uint16_t duty = 0; duty < 1000; duty += 10) {
        TIM2_CCR1 = duty;
        delay_ms(10);
    }
    for (uint16_t duty = 1000; duty > 0; duty -= 10) {
        TIM2_CCR1 = duty;
        delay_ms(10);
    }
}
*** End of File

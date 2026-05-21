/**
 * @file    main.c
 * @brief   STM32F103C8T6 分层架构 LED 闪烁示例
 * @note    CubeMX 生成框架 + 分层架构应用代码
 *
 *          分层调用链：
 *          main.c (应用层) → led_driver.c (驱动层) → hal_gpio_stm32.c (HAL 层) → STM32 GPIO (硬件层)
 */

#include "main.h"
#include "gpio.h"
#include "led_driver.h"

/**
 * @brief  系统时钟配置（CubeMX 风格）
 *         HSE 8MHz → PLL ×9 → HCLK 72MHz
 */
void SystemClock_Config(void);

int main(void)
{
    /* ===== 系统初始化（CubeMX 生成风格）===== */
    HAL_Init();                /* HAL 库初始化，配置 SysTick */
    SystemClock_Config();      /* 时钟树配置：HCLK = 72MHz */
    MX_GPIO_Init();            /* GPIO 初始化：PC13 板载 LED */

    /* ===== 应用逻辑（分层架构：仅调用驱动层接口）===== */
    while (1)
    {
        led_toggle();       /* LED 状态翻转 */
        HAL_Delay(500);     /* 延时 500ms，LED 闪烁周期 1s */
    }
}

/**
 * @brief  系统时钟配置
 *         HSE 8MHz → PLL ×9 → SYSCLK 72MHz
 *         APB1 = 36MHz, APB2 = 72MHz
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 配置 HSE 作为 PLL 时钟源 */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;  /* 8MHz × 9 = 72MHz */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* 配置总线时钟分频 */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;   /* HCLK = 72MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_SYSCLK_DIV2;   /* APB1 = 36MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_SYSCLK_DIV1;   /* APB2 = 72MHz */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief  错误处理函数
 */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif

/**
 * @file    main.c
 * @brief   STM32F103C8T6 串口命令行固件（命令模式）主流程
 * @note    USART1 中断逐字节接收 → 环形缓冲 → 主循环解析分发到函数指针命令表。
 *          rx_byte 单字节接收缓冲定义在 usart.c 内（全项目唯一定义）。
 */
#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "command.h"
#include <string.h>

void SystemClock_Config(void);

int main(void)
{
    HAL_Init();                           /* HAL 库初始化，配置 SysTick */
    SystemClock_Config();                 /* 时钟树：HSE 8MHz → PLL ×9 → 72MHz */
    MX_GPIO_Init();                       /* PC13/PA0 LED 输出初始化 */
    MX_USART1_UART_Init();                /* USART1 115200 8N1 + 启动中断接收 */

    const char *banner = "cmdshell ready, type 'help'\r\n";   /* 长度由 strlen 计算，避免魔数 */
    HAL_UART_Transmit(&huart1, (uint8_t *)banner, strlen(banner), 100);
    while (1) {
        cmd_process();                    /* 前台：解析并执行命令 */
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

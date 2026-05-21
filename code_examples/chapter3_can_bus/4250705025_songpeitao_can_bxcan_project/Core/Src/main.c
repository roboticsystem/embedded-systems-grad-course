#include "main.h"
#include "can_user.h"
#include "agv_chassis_demo.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_CAN_Init(void);

int main(void)
{
    uint32_t last_can_tick = 0U;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_CAN_Init();
    CAN_UserStart();

    AGV_CAN_SetTargetRpm(1U, 1200);
    AGV_CAN_SetTargetRpm(2U, 1200);
    AGV_CAN_SetTargetRpm(3U, 1200);
    AGV_CAN_SetTargetRpm(4U, 1200);

    while (1) {
        if ((HAL_GetTick() - last_can_tick) >= 10U) {
            last_can_tick = HAL_GetTick();
            AGV_CAN_10msTask();
        }

        if (!AGV_CAN_IsMotorFeedbackFresh(1U, 200U) ||
            !AGV_CAN_IsMotorFeedbackFresh(2U, 200U) ||
            !AGV_CAN_IsMotorFeedbackFresh(3U, 200U) ||
            !AGV_CAN_IsMotorFeedbackFresh(4U, 200U)) {
            AGV_CAN_StopAllMotors();
        }
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 |
                                  RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

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

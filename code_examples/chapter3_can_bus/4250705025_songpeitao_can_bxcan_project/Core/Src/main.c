#include "main.h"
#include "can_user.h"
#include "agv_chassis_demo.h"
#include "usart.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_CAN_Init(void);

static void APP_Log(const char *message);
static char *APP_AppendText(char *dst, const char *text);
static char *APP_AppendU32(char *dst, uint32_t value);
static char *APP_AppendI16(char *dst, int16_t value);
static void APP_PrintStatus(void);
#if CAN_SIMULATION_LOOPBACK || CAN_PICSIMLAB_SOFT_DEMO
static void APP_SendSimulatedFeedback(uint16_t position);
#endif

int main(void)
{
    uint32_t last_can_tick = 0U;
    uint32_t last_led_tick = 0U;
    uint32_t last_status_tick = 0U;
#if CAN_SIMULATION_LOOPBACK || CAN_PICSIMLAB_SOFT_DEMO
    uint32_t last_sim_feedback_tick = 0U;
    uint16_t sim_position = 0U;
#endif

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    APP_Log("\r\nUART1 TX PA9 9600 8N1 ready\r\n");
#if CAN_PICSIMLAB_SOFT_DEMO
    APP_Log("PicSimLab soft CAN demo: bxCAN is not emulated by QEMU\r\n");
    APP_Log("CAN init reference: Prescaler=4 BS1=6TQ BS2=1TQ Mode=LoopBack\r\n");
    APP_Log("Filter reference: StdId 0x210-0x21F -> FIFO0, IDE=0, RTR=0\r\n");
    APP_Log("TX/RX reference: TX 0x201-0x204, RX 0x211-0x214\r\n");
#elif CAN_SIMULATION_LOOPBACK
    APP_Log("PicSimLab test build: starting CAN loopback init\r\n");
#endif

#if !CAN_PICSIMLAB_SOFT_DEMO
    MX_CAN_Init();
    CAN_UserStart();
#endif

    APP_Log("SongPeitao CAN AGV demo start\r\n");
#if CAN_PICSIMLAB_SOFT_DEMO
    APP_Log("CAN mode: PICSIMLAB SOFTWARE LOOPBACK for screenshot\r\n");
    APP_Log("Bitrate reference: 32MHz / 4 / (1 + 6 + 1) = 1Mbps\r\n");
#elif CAN_SIMULATION_LOOPBACK
    APP_Log("CAN mode: LOOPBACK for PicSimLab screenshot\r\n");
    APP_Log("Bitrate: 32MHz / 4 / (1 + 6 + 1) = 1Mbps\r\n");
#else
    APP_Log("CAN mode: NORMAL for real CAN transceiver\r\n");
    APP_Log("Bitrate: 36MHz / 4 / (1 + 6 + 2) = 1Mbps\r\n");
#endif

    AGV_CAN_SetTargetRpm(1U, 1200);
    AGV_CAN_SetTargetRpm(2U, 1200);
    AGV_CAN_SetTargetRpm(3U, 1200);
    AGV_CAN_SetTargetRpm(4U, 1200);

    while (1) {
        if ((HAL_GetTick() - last_can_tick) >= 10U) {
            last_can_tick = HAL_GetTick();
#if !CAN_PICSIMLAB_SOFT_DEMO
            AGV_CAN_10msTask();
#endif
        }

#if CAN_SIMULATION_LOOPBACK || CAN_PICSIMLAB_SOFT_DEMO
        if ((HAL_GetTick() - last_sim_feedback_tick) >= 100U) {
            last_sim_feedback_tick = HAL_GetTick();
            sim_position += 10U;
            APP_SendSimulatedFeedback(sim_position);
        }
#endif

        if ((HAL_GetTick() - last_led_tick) >= 500U) {
            last_led_tick = HAL_GetTick();
            HAL_GPIO_TogglePin(CAN_STATUS_LED_GPIO_Port, CAN_STATUS_LED_Pin);
        }

        if ((HAL_GetTick() - last_status_tick) >= 1000U) {
            last_status_tick = HAL_GetTick();
            APP_PrintStatus();
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

#if CAN_SIMULATION_LOOPBACK
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
#else
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
#endif
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    HAL_GPIO_WritePin(CAN_STATUS_LED_GPIO_Port, CAN_STATUS_LED_Pin,
                      GPIO_PIN_SET);

    GPIO_InitStruct.Pin = CAN_STATUS_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(CAN_STATUS_LED_GPIO_Port, &GPIO_InitStruct);
}

static void APP_Log(const char *message)
{
    const uint8_t *data = (const uint8_t *)message;
    uint16_t len = 0U;

    while (message[len] != '\0') {
        len++;
    }

    (void)HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 100U);
}

static void APP_PrintStatus(void)
{
    char line[128];
    char *p = line;

#if CAN_PICSIMLAB_SOFT_DEMO
    p = APP_AppendText(p, "CAN SOFT LOOPBACK TX/RX OK id=0x211 rpm=");
#elif CAN_SIMULATION_LOOPBACK
    p = APP_AppendText(p, "CAN LOOPBACK TX/RX OK id=0x211 rpm=");
#else
    p = APP_AppendText(p, "CAN NORMAL RX status id=0x211 rpm=");
#endif
    p = APP_AppendI16(p, g_can_motor_feedback[0].rpm);
    p = APP_AppendText(p, " current=");
    p = APP_AppendI16(p, g_can_motor_feedback[0].current_ma);
    p = APP_AppendText(p, " pos=");
    p = APP_AppendU32(p, g_can_motor_feedback[0].position);
    p = APP_AppendText(p, " tick=");
    p = APP_AppendU32(p, HAL_GetTick());
    p = APP_AppendText(p, "\r\n");
    *p = '\0';

    (void)HAL_UART_Transmit(&huart1, (uint8_t *)line,
                            (uint16_t)(p - line), 100U);
}

#if CAN_SIMULATION_LOOPBACK || CAN_PICSIMLAB_SOFT_DEMO
static void APP_SendSimulatedFeedback(uint16_t position)
{
#if CAN_PICSIMLAB_SOFT_DEMO
    g_can_motor_feedback[0].rpm = 1200;
    g_can_motor_feedback[0].current_ma = 320;
    g_can_motor_feedback[0].position = position;
    g_can_motor_feedback[0].last_tick = HAL_GetTick();
    g_can_motor_feedback[1].rpm = 1210;
    g_can_motor_feedback[1].current_ma = 330;
    g_can_motor_feedback[1].position = position + 10U;
    g_can_motor_feedback[1].last_tick = HAL_GetTick();
    g_can_motor_feedback[2].rpm = 1190;
    g_can_motor_feedback[2].current_ma = 315;
    g_can_motor_feedback[2].position = position + 20U;
    g_can_motor_feedback[2].last_tick = HAL_GetTick();
    g_can_motor_feedback[3].rpm = 1205;
    g_can_motor_feedback[3].current_ma = 325;
    g_can_motor_feedback[3].position = position + 30U;
    g_can_motor_feedback[3].last_tick = HAL_GetTick();
#else
    (void)CAN_SendSimulatedMotorFeedback(1U, 1200, 320, position);
    (void)CAN_SendSimulatedMotorFeedback(2U, 1210, 330, position + 10U);
    (void)CAN_SendSimulatedMotorFeedback(3U, 1190, 315, position + 20U);
    (void)CAN_SendSimulatedMotorFeedback(4U, 1205, 325, position + 30U);
#endif
}
#endif

static char *APP_AppendText(char *dst, const char *text)
{
    while (*text != '\0') {
        *dst = *text;
        dst++;
        text++;
    }

    return dst;
}

static char *APP_AppendU32(char *dst, uint32_t value)
{
    char tmp[10];
    uint8_t i = 0U;

    if (value == 0U) {
        *dst = '0';
        return dst + 1;
    }

    while (value > 0U && i < sizeof(tmp)) {
        tmp[i] = (char)('0' + (value % 10U));
        value /= 10U;
        i++;
    }

    while (i > 0U) {
        i--;
        *dst = tmp[i];
        dst++;
    }

    return dst;
}

static char *APP_AppendI16(char *dst, int16_t value)
{
    int32_t wide = value;

    if (wide < 0) {
        *dst = '-';
        dst++;
        wide = -wide;
    }

    return APP_AppendU32(dst, (uint32_t)wide);
}

void Error_Handler(void)
{
#if CAN_SIMULATION_LOOPBACK
    APP_Log("ERROR: initialization stopped before main loop\r\n");
    while (1) {
        HAL_GPIO_TogglePin(CAN_STATUS_LED_GPIO_Port, CAN_STATUS_LED_Pin);
        HAL_Delay(250U);
    }
#else
    __disable_irq();
    while (1) {
    }
#endif
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif

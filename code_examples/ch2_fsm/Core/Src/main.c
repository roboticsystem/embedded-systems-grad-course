/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 状态机演示主程序
  * @author         : Zhou Qing
  * @date           : 2024
  ******************************************************************************
  */

#include "main.h"
#include "fsm.h"
#include "gpio.h"

/* 密码定义 */
#define PASSWORD_SEQUENCE {0, 1, 2}  /* PA0 -> PA1 -> PA2 */

/* 静态函数声明 */
static void SystemClock_Config(void);
static void process_key_input(void);

/**
  * @brief  主函数
  * @param  无
  * @retval 0
  */
int main(void) {
    /* 初始化HAL库 */
    HAL_Init();
    
    /* 配置系统时钟 */
    SystemClock_Config();
    
    /* 初始化GPIO */
    gpio_init();
    
    /* 初始化状态机 */
    fsm_init();

    /* 主循环 */
    while (1) {
        /* 处理按键输入 */
        process_key_input();
        
        /* 延时 */
        HAL_Delay(100);
    }
}

/**
  * @brief  处理按键输入
  * @param  无
  * @retval 无
  */
static void process_key_input(void) {
    static uint8_t password_index = 0;
    const uint8_t password[] = PASSWORD_SEQUENCE;
    uint8_t key_pressed = 0;
    uint8_t pressed_key = 0xFF;

    /* 检测按键按下 */
    if (key_pw0_read()) {
        pressed_key = 0;
        key_pressed = 1;
    } else if (key_pw1_read()) {
        pressed_key = 1;
        key_pressed = 1;
    } else if (key_pw2_read()) {
        pressed_key = 2;
        key_pressed = 1;
    }

    /* 如果有按键按下 */
    if (key_pressed && pressed_key != 0xFF) {
        /* 延时消抖 */
        HAL_Delay(50);
        
        if (pressed_key == password[password_index]) {
            /* 当前按键正确 */
            password_index++;
            
            if (password_index >= sizeof(password)) {
                /* 密码输入完成且正确 */
                fsm_process(EVENT_PASSWORD_OK);
                password_index = 0;
            }
        } else {
            /* 密码错误 */
            fsm_process(EVENT_PASSWORD_ERR);
            password_index = 0;
        }
        
        /* 等待按键释放 */
        while (key_pw0_read() || key_pw1_read() || key_pw2_read()) {
            HAL_Delay(10);
        }
    }
}

/**
  * @brief  系统时钟配置
  * @param  无
  * @retval 无
  */
static void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* 配置主PLL */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* 配置系统时钟 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/**
  * @brief  错误处理函数
  * @param  无
  * @retval 无
  */
void Error_Handler(void) {
    /* 用户可自定义错误处理 */
    while(1) {
    }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  断言失败处理函数
  * @param  file: 文件名
  * @param  line: 行号
  * @retval 无
  */
void assert_failed(uint8_t *file, uint32_t line) {
    /* 用户可自定义断言失败处理 */
}
#endif /* USE_FULL_ASSERT */
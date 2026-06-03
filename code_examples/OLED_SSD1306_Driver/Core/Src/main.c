/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define OLED_ADDR (0x3C << 1)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t data);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, char *str);
void OLED_ShowSensorData(float temp, float humi);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 6x8 字体表（ASCII 32~126 部分示例）
const uint8_t Font_6x8[95][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, // 32 空格
    {0x00,0x00,0x5F,0x00,0x00,0x00}, // 33 !
    {0x00,0x03,0x00,0x03,0x00,0x00}, // 34 "
    {0x14,0x3E,0x14,0x3E,0x14,0x00}, // 35 #
    {0x24,0x2A,0x7F,0x2A,0x12,0x00}, // 36 $
    {0x43,0x33,0x08,0x66,0x61,0x00}, // 37 %
    {0x36,0x49,0x55,0x22,0x50,0x00}, // 38 &
    {0x05,0x03,0x00,0x00,0x00,0x00}, // 39 '
    {0x00,0x1C,0x22,0x41,0x00,0x00}, // 40 (
    {0x00,0x41,0x22,0x1C,0x00,0x00}, // 41 )
    {0x14,0x08,0x3E,0x08,0x14,0x00}, // 42 *
    {0x08,0x08,0x3E,0x08,0x08,0x00}, // 43 +
    {0x00,0x50,0x30,0x00,0x00,0x00}, // 44 ,
    {0x08,0x08,0x08,0x08,0x08,0x00}, // 45 -
    {0x00,0x60,0x60,0x00,0x00,0x00}, // 46 .
    {0x20,0x10,0x08,0x04,0x02,0x00}, // 47 /
    {0x3E,0x51,0x49,0x45,0x3E,0x00}, // 48 0
    {0x00,0x42,0x7F,0x40,0x00,0x00}, // 49 1
    {0x42,0x61,0x51,0x49,0x46,0x00}, // 50 2
    {0x21,0x41,0x45,0x4B,0x31,0x00}, // 51 3
    {0x18,0x14,0x12,0x7F,0x10,0x00}, // 52 4
    {0x27,0x45,0x45,0x45,0x39,0x00}, // 53 5
    {0x3C,0x4A,0x49,0x49,0x30,0x00}, // 54 6
    {0x01,0x71,0x09,0x05,0x03,0x00}, // 55 7
    {0x36,0x49,0x49,0x49,0x36,0x00}, // 56 8
    {0x06,0x49,0x49,0x29,0x1E,0x00}, // 57 9
    {0x00,0x36,0x36,0x00,0x00,0x00}, // 58 :
    {0x00,0x56,0x36,0x00,0x00,0x00}, // 59 ;
    {0x08,0x14,0x22,0x41,0x00,0x00}, // 60 <
    {0x14,0x14,0x14,0x14,0x14,0x00}, // 61 =
    {0x00,0x41,0x22,0x14,0x08,0x00}, // 62 >
    {0x02,0x01,0x51,0x09,0x06,0x00}, // 63 ?
    {0x3E,0x41,0x5D,0x55,0x1E,0x00}, // 64 @
    {0x7E,0x11,0x11,0x11,0x7E,0x00}, // 65 A
    {0x7F,0x49,0x49,0x49,0x36,0x00}, // 66 B
    {0x3E,0x41,0x41,0x41,0x22,0x00}, // 67 C
    {0x7F,0x41,0x41,0x22,0x1C,0x00}, // 68 D
    {0x7F,0x49,0x49,0x49,0x41,0x00}, // 69 E
    {0x7F,0x09,0x09,0x09,0x01,0x00}, // 70 F
    {0x3E,0x41,0x49,0x49,0x7A,0x00}, // 71 G
    {0x7F,0x08,0x08,0x08,0x7F,0x00}, // 72 H
    {0x00,0x41,0x7F,0x41,0x00,0x00}, // 73 I
    {0x20,0x40,0x41,0x3F,0x01,0x00}, // 74 J
    {0x7F,0x08,0x14,0x22,0x41,0x00}, // 75 K
    {0x7F,0x40,0x40,0x40,0x40,0x00}, // 76 L
    {0x7F,0x02,0x0C,0x02,0x7F,0x00}, // 77 M
    {0x7F,0x04,0x08,0x10,0x7F,0x00}, // 78 N
    {0x3E,0x41,0x41,0x41,0x3E,0x00}, // 79 O
    {0x7F,0x09,0x09,0x09,0x06,0x00}, // 80 P
    {0x3E,0x41,0x51,0x21,0x5E,0x00}, // 81 Q
    {0x7F,0x09,0x19,0x29,0x46,0x00}, // 82 R
    {0x46,0x49,0x49,0x49,0x31,0x00}, // 83 S
    {0x01,0x01,0x7F,0x01,0x01,0x00}, // 84 T
    {0x3F,0x40,0x40,0x40,0x3F,0x00}, // 85 U
    {0x1F,0x20,0x40,0x20,0x1F,0x00}, // 86 V
    {0x7F,0x20,0x18,0x20,0x7F,0x00}, // 87 W
    {0x63,0x14,0x08,0x14,0x63,0x00}, // 88 X
    {0x07,0x08,0x70,0x08,0x07,0x00}, // 89 Y
    {0x61,0x51,0x49,0x45,0x43,0x00}, // 90 Z
    {0x00,0x7F,0x41,0x41,0x00,0x00}, // 91 [
    {0x02,0x04,0x08,0x10,0x20,0x00}, // 92 backslash
    {0x00,0x41,0x41,0x7F,0x00,0x00}, // 93 ]
    {0x04,0x02,0x01,0x02,0x04,0x00}, // 94 ^
    {0x40,0x40,0x40,0x40,0x40,0x00}, // 95 _
    {0x00,0x01,0x02,0x04,0x00,0x00}, // 96 `
    {0x20,0x54,0x54,0x54,0x78,0x00}, // 97 a
    {0x7F,0x48,0x44,0x44,0x38,0x00}, // 98 b
    {0x38,0x44,0x44,0x44,0x20,0x00}, // 99 c
    {0x38,0x44,0x44,0x48,0x7F,0x00}, // 100 d
    {0x38,0x54,0x54,0x54,0x18,0x00}, // 101 e
    {0x08,0x7E,0x09,0x01,0x02,0x00}, // 102 f
    {0x18,0xA4,0xA4,0xA4,0x7C,0x00}, // 103 g
    {0x7F,0x08,0x04,0x04,0x78,0x00}, // 104 h
    {0x00,0x44,0x7D,0x40,0x00,0x00}, // 105 i
    {0x20,0x40,0x44,0x3D,0x00,0x00}, // 106 j
    {0x7F,0x10,0x28,0x44,0x00,0x00}, // 107 k
    {0x00,0x41,0x7F,0x40,0x00,0x00}, // 108 l
    {0x7C,0x04,0x18,0x04,0x78,0x00}, // 109 m
    {0x7C,0x08,0x04,0x04,0x78,0x00}, // 110 n
    {0x38,0x44,0x44,0x44,0x38,0x00}, // 111 o
    {0x7C,0x14,0x14,0x14,0x08,0x00}, // 112 p
    {0x08,0x14,0x14,0x18,0x7C,0x00}, // 113 q
    {0x7C,0x08,0x04,0x04,0x08,0x00}, // 114 r
    {0x48,0x54,0x54,0x54,0x20,0x00}, // 115 s
    {0x04,0x3F,0x44,0x40,0x20,0x00}, // 116 t
    {0x3C,0x40,0x40,0x20,0x7C,0x00}, // 117 u
    {0x1C,0x20,0x40,0x20,0x1C,0x00}, // 118 v
    {0x3C,0x60,0x30,0x60,0x3C,0x00}, // 119 w
    {0x44,0x28,0x10,0x28,0x44,0x00}, // 120 x
    {0x1C,0xA0,0xA0,0xA0,0x7C,0x00}, // 121 y
    {0x44,0x64,0x54,0x4C,0x44,0x00}  // 122 z
};

void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 2, HAL_MAX_DELAY);
}

void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = {0x40, data};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 2, HAL_MAX_DELAY);
}

void OLED_Init(void)
{
    HAL_Delay(100);
    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0xD5); OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8); OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xD3); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D); OLED_WriteCmd(0x14);
    OLED_WriteCmd(0x20); OLED_WriteCmd(0x00);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0xDA); OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81); OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9); OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB); OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0xAF);
}

void OLED_Clear(void)
{
    OLED_WriteCmd(0x21); OLED_WriteCmd(0x00); OLED_WriteCmd(0x7F);
    OLED_WriteCmd(0x22); OLED_WriteCmd(0x00); OLED_WriteCmd(0x07);
    for (uint16_t i = 0; i < 1024; i++)
    {
        OLED_WriteData(0x00);
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch)
{
    if (x > 122 || y > 7) return;
    OLED_WriteCmd(0xB0 + y);
    OLED_WriteCmd(0x00 + (x & 0x0F));
    OLED_WriteCmd(0x10 + ((x >> 4) & 0x0F));
    for (uint8_t i = 0; i < 6; i++)
    {
        OLED_WriteData(Font_6x8[ch - 32][i]);
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, char *str)
{
    while (*str)
    {
        OLED_ShowChar(x, y, *str);
        x += 6;
        if (x > 122)
        {
            x = 0;
            y++;
            if (y > 7) break;
        }
        str++;
    }
}

void OLED_ShowSensorData(float temp, float humi)
{
    char buf[20];
    OLED_Clear();
    sprintf(buf, "Temp: %.1f C", temp);
    OLED_ShowString(0, 0, buf);
    sprintf(buf, "Hum:  %.1f %%", humi);
    OLED_ShowString(0, 2, buf);
    OLED_ShowString(0, 4, "SSD1306 OK");
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();
  /* USER CODE END 2 */
  float temperature = 23.5;
  float humidity = 58.0;
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  OLED_ShowSensorData(temperature, humidity);
	  HAL_Delay(2000);   // 每2秒刷新一次
	  temperature += 0.5;
	  humidity += 0.5;
	  if (temperature > 40) temperature = 20;
	  if (humidity > 80) humidity = 40;
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

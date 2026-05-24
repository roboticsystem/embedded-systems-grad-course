---
number headings: first-level 2, start-at 7
---

## 7 第 7 章 传感器接口编程

> 传感器是嵌入式系统感知外部世界的"眼睛和耳朵"。本章介绍超声波测距、温湿度检测、ADC 模拟采样和红外传感器等常用传感器在 STM32 上的接口编程方法，所有实验均可在 PicSimlab 中完成仿真验证。

### 7.1 本章知识导图

```plantuml
@startmindmap
skinparam mindmapNodeBackgroundColor<<root>>    #1565C0
skinparam mindmapNodeFontColor<<root>>          white
skinparam mindmapNodeBackgroundColor<<l1>>      #1976D2
skinparam mindmapNodeFontColor<<l1>>            white
skinparam ArrowColor                            #90CAF9
skinparam mindmapNodeBorderColor                #90CAF9

* 第7章 传感器接口编程
** 传感器分类
*** 模拟传感器（ADC采样）
*** 数字传感器（GPIO/协议）
** 超声波测距 HC-SR04
*** 工作原理（Trig/Echo）
*** 定时器输入捕获
*** 距离计算公式
** 温湿度传感器 DHT11
*** 单总线协议
*** 时序解析
*** 数据校验
** ADC 模拟采样
*** ADC 基本原理
*** STM32 ADC 配置
*** 多通道扫描与DMA
** 红外传感器
*** 数字输出型（障碍检测）
*** 模拟输出型（距离感知）
@endmindmap
```

**图 7-1** 本章知识导图：传感器分类与四种常用传感器的接口编程。
<!-- fig:ch7-1 本章知识导图：传感器分类与四种常用传感器的接口编程。 -->

### 7.2 传感器分类与接口概述

传感器按输出信号类型可分为两大类：

**表 7-1** 传感器分类
<!-- tab:ch7-1 传感器分类 -->

| 类型 | 输出信号 | STM32 接口 | 典型传感器 |
|------|---------|-----------|-----------|
| 模拟传感器 | 连续电压（0~3.3V） | ADC | 热敏电阻、光敏电阻、气体传感器 |
| 数字传感器（电平型）| 高/低电平 | GPIO 输入 | 红外避障、限位开关、霍尔传感器 |
| 数字传感器（协议型）| 特定通信协议 | GPIO/I2C/SPI | DHT11（单总线）、BMP280（I2C） |
| 数字传感器（脉冲型）| 脉冲宽度/频率 | 定时器输入捕获 | HC-SR04（超声波）、编码器 |

```bob
  ┌─────────────────┐
  │    外部物理量    │
  │  温度/距离/光照  │
  └────────┬────────┘
           │ 传感器转换
           ▼
  ┌────────────────────────────────────────┐
  │          传感器输出信号                  │
  ├──────────┬──────────┬──────────────────┤
  │ 模拟电压 │ 数字电平 │ 数字脉冲/协议    │
  │ 0~3.3V   │ HIGH/LOW │ PWM/单总线/I2C   │
  ├──────────┼──────────┼──────────────────┤
  │  ADC     │  GPIO    │ TIM/GPIO/I2C     │
  │  采样    │  读取    │ 捕获/解析        │
  └──────────┴──────────┴──────────────────┘
           │ STM32 处理
           ▼
  ┌─────────────────┐
  │  数据 → 应用层  │
  │  显示/控制/上报  │
  └─────────────────┘
```

**图 7-2** 传感器信号从物理量到 STM32 数据处理的转换链路。
<!-- fig:ch7-2 传感器信号从物理量到 STM32 数据处理的转换链路。 -->

---

### 7.3 超声波测距传感器（HC-SR04）

HC-SR04 是最常用的超声波测距模块，可测量 2cm~400cm 范围内的距离，广泛用于障碍检测和液位监测。

#### 7.3.1 工作原理

```bob
     STM32                    HC-SR04
  ┌──────────┐            ┌──────────────┐
  │  Trig(PA0)├───────────►│  Trig        │
  │          │            │              │───── 超声波发射 ─────►
  │  Echo(PA1)│◄───────────┤  Echo        │
  │          │            │              │◄──── 超声波回波 ─────
  │  VCC     ├───────────►│  VCC (5V)    │
  │  GND     ├───────────►│  GND         │
  └──────────┘            └──────────────┘
```

**图 7-3** HC-SR04 与 STM32 的连接示意图。
<!-- fig:ch7-3 HC-SR04 与 STM32 的连接示意图。 -->

**测距流程：**

1. STM32 向 Trig 引脚发送 ≥10μs 的高电平脉冲
2. HC-SR04 自动发射 8 个 40kHz 超声波脉冲
3. 发射后 Echo 引脚拉高，等待回波
4. 收到回波后 Echo 拉低，高电平持续时间即为超声波往返时间

**距离计算公式：**

$$d = \frac{v \times t}{2} = \frac{340 \times t_{echo}}{2} \text{ (m)}$$

其中 $t_{echo}$ 为 Echo 高电平持续时间（秒），声速取 340 m/s。

#### 7.3.2 STM32 实现（定时器输入捕获）

使用定时器输入捕获测量 Echo 脉冲宽度：

**CubeMX 配置：**

- PA0 → GPIO_Output（Trig）
- PA1 → TIM2_CH2（Echo，输入捕获模式）
- TIM2 PSC = 71，ARR = 65535（计数周期 1μs，最大 65.535ms）

```c
/* 超声波测距驱动 */
#include "main.h"

static volatile uint32_t echo_start = 0;
static volatile uint32_t echo_end   = 0;
static volatile uint8_t  capture_done = 0;

/* 发送 Trig 脉冲 */
void HC_SR04_Trigger(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    uint32_t tick = SysTick->VAL;
    while ((tick - SysTick->VAL) < 720) {}   /* 约 10us @72MHz */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
}

/* 定时器输入捕获回调 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
        if (!capture_done) {
            if (echo_start == 0) {
                echo_start = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
                /* 切换为下降沿捕获 */
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
                                               TIM_INPUTCHANNELPOLARITY_FALLING);
            } else {
                echo_end = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
                capture_done = 1;
                /* 恢复上升沿捕获 */
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2,
                                               TIM_INPUTCHANNELPOLARITY_RISING);
            }
        }
    }
}

/* 获取距离（单位：cm） */
float HC_SR04_GetDistance(void)
{
    echo_start = 0;
    echo_end   = 0;
    capture_done = 0;

    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
    HC_SR04_Trigger();

    uint32_t timeout = HAL_GetTick();
    while (!capture_done && (HAL_GetTick() - timeout < 50)) {}

    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);

    if (!capture_done) return -1.0f;  /* 超时 */

    uint32_t pulse_us = (echo_end >= echo_start)
                        ? (echo_end - echo_start)
                        : (65536 - echo_start + echo_end);
    return (float)pulse_us * 0.034f / 2.0f;  /* cm */
}
```

> 在 PicSimlab 中可使用 Ultrasonic 虚拟组件模拟 HC-SR04，通过滑块调节模拟距离值。

---

### 7.4 温湿度传感器（DHT11）

DHT11 是一款低成本的温湿度传感器，通过单总线协议与 MCU 通信，适用于农业温室环境监测等场景。

#### 7.4.1 单总线协议时序

DHT11 仅需一根数据线（DATA），通信流程如下：

1. **MCU 发送起始信号**：拉低 DATA ≥18ms，然后拉高 20~40μs
2. **DHT11 响应**：拉低 80μs → 拉高 80μs
3. **数据传输**：40 位数据（湿度整数+小数 + 温度整数+小数 + 校验和），每位以脉冲宽度编码：
   - "0"：低电平 50μs + 高电平 26~28μs
   - "1"：低电平 50μs + 高电平 70μs
4. **校验**：校验和 = 湿度整数 + 湿度小数 + 温度整数 + 温度小数

#### 7.4.2 STM32 实现

```c
/* DHT11 驱动 */
#include "main.h"

#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_PIN_0

typedef struct {
    uint8_t humidity;       /* 湿度整数部分 */
    uint8_t temperature;    /* 温度整数部分 */
    uint8_t valid;          /* 数据是否有效 */
} DHT11_Data;

/* 微秒延时（基于 DWT） */
static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks) {}
}

/* 设置引脚方向 */
static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin   = DHT11_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &g);
}

static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin  = DHT11_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &g);
}

/* 读取一个字节 */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {}
        delay_us(40);  /* 超过 28us 则为 '1' */
        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
            byte |= (1 << (7 - i));
        }
        while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {}
    }
    return byte;
}

/* 读取完整数据 */
DHT11_Data DHT11_Read(void)
{
    DHT11_Data data = {0};
    uint8_t buf[5];

    /* 发送起始信号 */
    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);  /* 拉低 20ms */
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(30);

    /* 等待 DHT11 响应 */
    DHT11_SetInput();
    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) return data;

    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET) {}
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {}

    /* 读取 5 字节数据 */
    for (int i = 0; i < 5; i++) {
        buf[i] = DHT11_ReadByte();
    }

    /* 校验 */
    if (buf[4] == (uint8_t)(buf[0] + buf[1] + buf[2] + buf[3])) {
        data.humidity    = buf[0];
        data.temperature = buf[2];
        data.valid       = 1;
    }
    return data;
}
```

---

### 7.5 ADC 模拟采样

STM32F103 内置 12 位 ADC，最大采样率 1 MSPS，可将 0~3.3V 模拟电压转换为 0~4095 的数字值。

#### 7.5.1 ADC 基本原理

$$V_{input} = \frac{ADC\_Value}{4095} \times V_{ref} = \frac{ADC\_Value}{4095} \times 3.3 \text{ (V)}$$

**CubeMX 配置：**

- PA0 → ADC1_IN0
- ADC1：12 位分辨率，连续转换模式
- 采样时间：239.5 cycles（采样时间越长精度越高）

#### 7.5.2 单通道轮询采样

```c
/* ADC 单通道采样 */
uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return value;
}

float ADC_ReadVoltage(void)
{
    return (float)ADC_Read() / 4095.0f * 3.3f;
}
```

#### 7.5.3 多通道 DMA 扫描

当需要同时采集多路模拟信号（如温度+光照+土壤湿度）时，使用 DMA 扫描模式避免 CPU 等待：

```c
/* DMA 多通道采样 */
#define ADC_CHANNELS 3
static uint16_t adc_buf[ADC_CHANNELS];

/* 启动 DMA 连续采样 */
void ADC_DMA_Start(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, ADC_CHANNELS);
}

/* 读取各通道电压 */
float ADC_GetChannel(uint8_t ch)
{
    if (ch >= ADC_CHANNELS) return 0.0f;
    return (float)adc_buf[ch] / 4095.0f * 3.3f;
}
```

---

### 7.6 红外传感器

红外传感器利用红外线的反射特性检测物体是否存在或测量物体距离，是嵌入式系统中最常用的低成本检测方案之一。本节介绍数字输出型和模拟输出型两类红外传感器的接口编程方法。

#### 7.6.1 学习目标

完成本节学习后，你应当能够：

- 理解红外传感器的工作原理和分类
- 掌握数字输出型红外传感器的 GPIO 接口编程
- 掌握模拟输出型红外传感器的 ADC 采样与距离计算
- 能够在 PicSimlab 中搭建红外传感器仿真电路

#### 7.6.2 工作原理

红外传感器由红外发射管和红外接收管组成，通过检测发射光经物体反射后的强度来判断物体是否存在或距离远近。

```bob
      ┌──────────────────────────────────────────────────────────────┐
      │                       红外传感器工作原理                      │
      ├──────────────────────────────────────────────────────────────┤
      │                                                              │
      │    发射管        目标物体        接收管        输出信号      │
      │    ┌───┐         ┌───┐         ┌───┐         ┌──────┐      │
      │    │IR │  ~~~~>  │   │  <~~~~~ │RT │    ────>│ 高/低 │      │
      │    │发射│         │物体│         │接收│         │ 电平  │      │
      │    └───┘         └───┘         └───┘         └──────┘      │
      │      │                              │                              │
      │      └────────── 反射光 ────────────┘              │
      │                                                              │
      │    无物体：反射弱 → 输出低电平（数字型）/ 低电压（模拟型）    │
      │    有物体：反射强 → 输出高电平（数字型）/ 高电压（模拟型）    │
      └──────────────────────────────────────────────────────────────┘
```

**图 7-6** 红外传感器工作原理示意图。
<!-- fig:ch7-6 红外传感器工作原理示意图。 -->

根据输出信号类型，红外传感器可分为两类：

**表 7-2** 红外传感器分类与特点
<!-- tab:ch7-2 红外传感器分类与特点 -->

| 类型 | 工作原理 | 输出信号 | STM32 接口 | 典型型号 | 检测距离 |
|------|---------|---------|-----------|---------|---------|
| 数字输出型 | 反射光强度阈值比较 | 高/低电平 | GPIO 输入 | TCRT5000 | ~10 mm |
| 模拟输出型 | 反射光强度线性转换 | 0~3.3V 模拟电压 | ADC 采样 | GP2Y0A21 | 10~80 cm |

数字输出型传感器内部集成了电压比较器，当反射光强超过阈值时输出状态翻转，适合简单的有/无检测。模拟输出型则输出与距离成反比的连续电压信号，可实现距离估算。

---

#### 7.6.3 数字输出型红外传感器（TCRT5000）

TCRT5000 是最常见的数字输出型红外传感器，由红外发射管、光敏接收管和比较器电路组成，检测距离约 1~10 mm。

**CubeMX 配置步骤：**

1. 打开 CubeMX，选择 STM32F103C8Tx 芯片
2. 在引脚配置图中找到 PB0，将其配置为 `GPIO_Input`
3. 在 GPIO 模式设置中，勾选 `GPIO Pull-up`（上拉模式）
4. 可选：配置外部中断 - 在 PB0 的 GPIO EXTI 模式中设置为上升沿/下降沿触发
5. 点击 `Generate Code` 生成工程代码

![CubeMX GPIO 配置](assets/ch7/cubemx-gpio-input.png){ width="80%" }

**图 7-8** CubeMX 配置 PB0 为 GPIO 输入模式（上拉）。
<!-- fig:ch7-8 CubeMX 配置 PB0 为 GPIO 输入模式（上拉）。 -->

**连接示意图：**

```bob
     STM32F103                  TCRT5000
  ┌────────────┐            ┌──────────────┐
  │            │            │   ┌──────┐   │
  │    3.3V    ├────────────►│ VCC│      │   │
  │            │            │   │  IR  │   │
  │    PB0     ├────────────►│OUT│      │   │
  │  (GPIO_IN) │            │   │ 模块 │   │
  │            │            │   │      │   │
  │    GND     ├────────────►│GND│      │   │
  │            │            │   └──────┘   │
  └────────────┘            └──────────────┘
```

**图 7-7** TCRT5000 与 STM32 的连接示意图。
<!-- fig:ch7-7 TCRT5000 与 STM32 的连接示意图。 -->

**STM32 驱动代码：**

```c
/* 数字输出型红外传感器驱动 */
#include "main.h"

#define IR_OUT_PORT  GPIOB
#define IR_OUT_PIN   GPIO_PIN_0

/**
 * @brief 初始化红外传感器引脚
 * @note  在 CubeMX 中配置为输入模式 + 上拉
 */
void IR_Digital_Init(void)
{
    /* CubeMX 已完成 GPIO 配置，此处仅作说明 */
}

/**
 * @brief 检测是否有障碍物
 * @retval 1 表示检测到障碍物，0 表示无障碍物
 * @note  TCRT5000 检测到物体时输出低电平
 */
uint8_t IR_Digital_IsObstacle(void)
{
    return (HAL_GPIO_ReadPin(IR_OUT_PORT, IR_OUT_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}

/**
 * @brief 红外传感器轮询检测示例
 */
void IR_Digital_PollingExample(void)
{
    if (IR_Digital_IsObstacle()) {
        /* 检测到障碍物，执行相应动作 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  /* 点亮 LED */
    } else {
        /* 无障碍物 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);  /* 熄灭 LED */
    }
}
```

**中断模式实现（可选）：**

当需要实时响应障碍物时，可配置 GPIO 外部中断：

```c
/* 外部中断回调函数 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == IR_OUT_PIN) {
        if (IR_Digital_IsObstacle()) {
            /* 障碍物进入检测区 */
        } else {
            /* 障碍物离开检测区 */
        }
    }
}
```

---

#### 7.6.4 模拟输出型红外传感器（GP2Y0A21）

GP2Y0A21 是夏普公司生产的模拟输出型红外距离传感器，输出电压与距离成非线性反比关系，检测范围 10~80 cm。

**CubeMX 配置步骤：**

1. 打开 CubeMX，选择 STM32F103C8Tx 芯片
2. 在引脚配置图中找到 PA0，将其配置为 `ADC1_IN0`
3. 在 ADC1 配置中：
   - 设置 Clock Prescaler 为 `PCLK2 div 4`
   - 设置 Resolution 为 `12 Bits`
   - 设置 Data Alignment 为 `Right alignment`
   - 在 ADC_Regular_ConversionMode 中设置 Continuous Conversion Mode 为 `Enable`
   - 设置 Sampling Time 为 `239.5 Cycles`
4. 点击 `Generate Code` 生成工程代码

![CubeMX ADC 配置](assets/ch7/cubemx-adc-config.png){ width="80%" }

**图 7-9** CubeMX 配置 PA0 为 ADC1_IN0 及参数设置。
<!-- fig:ch7-9 CubeMX 配置 PA0 为 ADC1_IN0 及参数设置。 -->

**距离-电压特性：**

GP2Y0A21 的输出电压与距离呈非线性关系，近似公式为：

$$d = \frac{27.86}{V_{out} - 0.42} \text{ (cm)}$$

其中 $V_{out}$ 为传感器输出电压（V），有效测量范围约为 10~80 cm。

**STM32 驱动代码：**

```c
/* 模拟输出型红外传感器驱动 */
#include "main.h"

#define IR_ADC_CHANNEL  0

/**
 * @brief 读取红外传感器 ADC 值
 * @retval ADC 转换值（0~4095）
 */
uint16_t IR_Analog_ReadRaw(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return adc_value;
}

/**
 * @brief 读取红外传感器输出电压
 * @retval 电压值（V）
 */
float IR_Analog_ReadVoltage(void)
{
    uint16_t adc_value = IR_Analog_ReadRaw();
    return (float)adc_value / 4095.0f * 3.3f;
}

/**
 * @brief 计算距离
 * @retval 距离值（cm），超出测量范围返回 -1
 * @note  使用经验公式：d = 27.86 / (Vout - 0.42)
 */
float IR_Analog_GetDistance(void)
{
    float voltage = IR_Analog_ReadVoltage();

    /* 检查电压是否在有效范围内 */
    if (voltage < 0.5f || voltage > 2.7f) {
        return -1.0f;  /* 超出测量范围 */
    }

    /* 经验公式计算距离 */
    float distance = 27.86f / (voltage - 0.42f);

    /* 限制在有效测量范围内 */
    if (distance < 10.0f) distance = 10.0f;
    if (distance > 80.0f) distance = 80.0f;

    return distance;
}
```

**距离测量示例：**

```c
/* 距离测量与阈值判断 */
void IR_Analog_DistanceExample(void)
{
    float distance = IR_Analog_GetDistance();

    if (distance < 0) {
        /* 超出测量范围 */
    } else if (distance < 30.0f) {
        /* 距离小于 30cm，近距离告警 */
    } else {
        /* 正常距离范围 */
    }
}
```

---

#### 7.6.5 PicSimlab 仿真实验

PicSimlab 提供了虚拟红外传感器组件，可方便地进行接口编程验证。

**实验步骤：**

1. **启动 PicSimlab 并选择开发板**

   ![PicSimlab 启动界面](assets/ch7/picsimlab-start.png){ width="80%" }

   **图 7-10** PicSimlab 主界面，选择 STM32F103C8T6（Blue Pill）开发板。
   <!-- fig:ch7-10 PicSimlab 主界面，选择 STM32F103C8T6（Blue Pill）开发板。 -->

2. **配置虚拟传感器组件**

   在 PicSimlab 中添加以下组件（通过 Pinout Configuration 配置引脚映射）：

   - **Push Buttons**（Discrete 分类）→ 映射到 **PB0**，模拟数字红外传感器
   - **Potentiometers**（Discrete 分类）→ 映射到 **PA0**，模拟模拟红外传感器
   - **LED**（Discrete 分类）→ 映射到 **PA5**，状态指示

   ![PicSimlab 组件配置](assets/ch7/picsimlab-ir-config.png){ width="80%" }

   **图 7-11** PicSimlab 组件引脚映射配置界面。
   <!-- fig:ch7-11 PicSimlab 组件引脚映射配置界面。 -->

3. **加载固件并运行**

   - 点击 `Load Hex` 按钮，选择编译生成的 .hex 文件
   - 点击运行按钮（绿色三角形）

4. **观察现象**

   ![PicSimlab 运行效果](assets/ch7/picsimlab-result.png){ width="80%" }

   **图 7-12** 仿真运行效果，按下 Push Button 时 LED 点亮。
   <!-- fig:ch7-12 仿真运行效果，按下 Push Button 时 LED 点亮。 -->

**预期结果：**

- **数字型**：当障碍物进入检测范围时，LED 状态翻转
- **模拟型**：ADC 采样值随距离增加而减小

---

#### 7.6.6 应用场景举例

红外传感器在农业信息化和工业自动化中有广泛应用。

**农产品计数：**

在农产品分拣传送带上，利用红外传感器检测通过的产品数量。

```c
/* 农产品计数示例 */
static volatile uint32_t product_count = 0;

void Product_CountingTask(void)
{
    static uint8_t last_state = 0;
    uint8_t current_state = IR_Digital_IsObstacle();

    /* 下降沿计数（物体进入检测区） */
    if (last_state == 0 && current_state == 1) {
        product_count++;
    }
    last_state = current_state;
}
```

**传送带物体检测：**

检测传送带上是否有物体存在，用于自动启停控制。

```c
/* 传送带控制示例 */
void Conveyor_ControlTask(void)
{
    if (IR_Digital_IsObstacle()) {
        /* 有物体，启动传送带 */
        Motor_Start();
    } else {
        /* 无物体，停止传送带 */
        Motor_Stop();
    }
}
```

**液位监测：**

利用模拟型红外传感器测量储液罐液位高度。

```c
/* 液位监测示例 */
void Tank_LevelMonitor(void)
{
    float distance = IR_Analog_GetDistance();
    float tank_height = 50.0f;  /* 罐高 50cm */

    if (distance > 0) {
        float liquid_level = tank_height - distance;

        if (liquid_level < 10.0f) {
            /* 液位过低，触发报警 */
            Alarm_Activate();
        }
    }
}
```

---

#### 7.6.7 本节小结

本节介绍了红外传感器的接口编程方法：

- **数字输出型**（TCRT5000）：通过 GPIO 读取高低电平判断障碍物有无，适用于简单的存在性检测
- **模拟输出型**（GP2Y0A21）：通过 ADC 采样输出电压并转换为距离值，可实现距离估算
- **PicSimlab 仿真**：可使用虚拟 Ir 组件进行接口验证，无需实际硬件

红外传感器成本低、接口简单，是嵌入式系统中最常用的传感器之一，广泛应用于物体检测、计数、测距等场景。

---

### 7.7 本章小结

本章介绍了嵌入式系统中四类常用传感器的接口编程方法：

- **超声波 HC-SR04**：定时器输入捕获测量 Echo 脉冲宽度，计算距离
- **温湿度 DHT11**：单总线协议时序解析，40 位数据读取与校验
- **ADC 模拟采样**：12 位 ADC 单通道轮询与多通道 DMA 扫描
- **红外传感器**：数字输出型 GPIO 读取 + 模拟输出型 ADC 采样与距离计算

这些传感器构成了嵌入式系统"输入"环节的核心，为后续的数据处理、显示和控制提供原始数据源。

---

### 7.8 习题

1. 说明 HC-SR04 的测距原理，写出距离计算公式。
2. DHT11 单总线协议如何区分数据位 "0" 和 "1"？
3. STM32 ADC 的分辨率为 12 位，当 ADC 值为 2048 时对应的电压是多少？
4. 设计一个农业温室环境监测方案，需要采集温度、湿度和土壤含水量（模拟量），说明传感器选型和 STM32 接口配置。
5. 比较轮询采样与 DMA 采样的优缺点，说明在多传感器场景下为什么推荐 DMA 方式。

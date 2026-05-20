## 1 STM32 定时器 PWM 呼吸灯设计与实现

> 本文以 STM32F103C8T6（Blue Pill）为硬件平台，系统讲解定时器 PWM 输出原理，通过 CubeMX 图形化配置生成初始化代码，在 CubeIDE 中实现呼吸灯应用，并在 PicSimlab 仿真环境中验证 PWM 波形与呼吸效果。

### 1.1 学习目标

完成本章学习后，读者应能够：

1. 理解 STM32 定时器的基本架构与 PWM 工作原理
2. 掌握 PWM 频率、占空比的计算方法与参数配置
3. 熟练使用 CubeMX 配置 TIM2 通道输出 1kHz PWM 信号
4. 编写 HAL 库代码实现占空比动态渐变的呼吸灯效果
5. 在 PicSimlab 仿真环境中加载固件、观察 PWM 波形并分析结果

---

### 1.2 知识点概述

#### 1.2.1 定时器在 STM32 中的地位

STM32F103 系列微控制器内置多个定时器，是嵌入式系统中最灵活的外设之一。定时器可用于产生精确的时间基准、PWM 信号、输入捕获、编码器接口等多种功能。

**表 1-1** STM32F103 定时器资源概览
<!-- tab:ch1-1 STM32F103 定时器资源概览 -->

| 定时器 | 类型 | 位宽 | 通道数 | 挂载总线 | 典型用途 |
|--------|------|------|--------|----------|----------|
| TIM1 | 高级 | 16 位 | 4 | APB2 | 互补 PWM、死区控制 |
| TIM2 | 通用 | 32 位 | 4 | APB1 | PWM 输出、输入捕获 |
| TIM3 | 通用 | 16 位 | 4 | APB1 | PWM 输出、编码器 |
| TIM4 | 通用 | 16 位 | 4 | APB1 | 通用定时 |

本文选用 **TIM2 通道 1（TIM2_CH1）** 输出 PWM 信号，对应引脚 **PA0**。TIM2 是 32 位通用定时器，具有 4 个独立通道，挂载在 APB1 总线上，时钟频率为 72MHz。

#### 1.2.2 PWM 的基本概念

**PWM（Pulse Width Modulation，脉宽调制）** 是一种通过改变方波高电平持续时间来等效调节平均电压的技术。PWM 广泛应用于 LED 亮度调节、电机转速控制、电源管理等嵌入式场景。

```bob
  PWM 信号波形示意
  占空比 = T_high / T_total × 100%

  100% 占空比（最亮）
      ┌───────────────────────┐
      │                       │
   ───┘                       └─────────

   50% 占空比（半亮）
      ┌────────┐              ┌────────┐
      │        │              │        │
   ───┘        └──────────────┘        └────

   25% 占空比（较暗）
      ┌──┐                    ┌──┐
      │  │                    │  │
   ───┘  └────────────────────┘  └──────────

   0% 占空比（熄灭）"
   ──────────────────────────────────────────
```

**图 1-1** PWM 信号波形与占空比关系示意图：占空比越高，LED 等效平均电压越大，亮度越亮。
<!-- fig:ch1-1 PWM 信号波形与占空比关系示意图：占空比越高，LED 等效平均电压越大，亮度越亮。 -->

PWM 的三个核心参数：

**表 1-2** PWM 核心参数
<!-- tab:ch1-2 PWM 核心参数 -->

| 参数 | 符号 | 含义 | 计算公式 |
|------|------|------|----------|
| PWM 频率 | $f_{PWM}$ | 每秒周期数 | $f_{PWM} = f_{clk} / ((PSC+1) \times (ARR+1))$ |
| 占空比 | Duty | 高电平占比 | $Duty = CCR / (ARR + 1) \times 100\%$ |
| 分辨率 | Res | 最小占空比步进 | $Res = 1 / (ARR + 1) \times 100\%$ |

其中 $f_{clk}$ 为定时器输入时钟，PSC 为预分频器值，ARR 为自动重装载值，CCR 为捕获/比较寄存器值。

---

### 1.3 定时器 PWM 工作原理

#### 1.3.1 定时器信号通路

STM32 定时器产生 PWM 信号的核心信号通路如下：

```bob
   APB1 总线时钟   Prescaler       计数器          ARR 
      |               |              |              |
      v               v              v              v
  +--------+    +-------------+    +----------+    +-------------+
  | 72 MHz |───→|  PSC = 71   |───→|  1 MHz   |───→|   ARR = 999 |
  +--------+    |  ÷ (PSC+1)  |    | 计数时钟 |    |  周期 = 1ms |
                +-------------+    +----+-----+    +-------------+
                                        |
                                        v
                                 +--------------+
                                 |  CCR = 500   |
                                 |  占空比 50%  |
                                 +--------------+
```

**图 1-2** 定时器 PWM 信号通路：APB1 时钟经预分频后进入计数器，计数器与 ARR 比较产生周期，与 CCR 比较产生占空比。
<!-- fig:ch1-2 定时器 PWM 信号通路：APB1 时钟经预分频后进入计数器，计数器与 ARR 比较产生周期，与 CCR 比较产生占空比。 -->

信号通路中各环节的说明：

**表 1-3** 定时器信号通路各环节说明
<!-- tab:ch1-3 定时器信号通路各环节说明 -->

| 环节 | 寄存器 | 功能 | 本例配置 |
|------|--------|------|----------|
| 时钟源 | RCC | APB1 总线时钟 | 72 MHz |
| 预分频 | PSC | 将时钟分频为计数时钟 | PSC = 71 → 1 MHz |
| 计数器 | CNT | 从 0 递增计数到 ARR | 0 → 999 循环 |
| 自动重装 | ARR | 决定 PWM 周期 | ARR = 999 → 1 kHz |
| 捕获比较 | CCR | 决定高电平持续时间 | 动态变化 0~999 |

#### 1.3.2 PWM 模式 1 的工作机理

STM32 定时器支持两种 PWM 模式，本文使用 **PWM 模式 1**：

**表 1-4** PWM 模式对比
<!-- tab:ch1-4 PWM 模式对比 -->

| 模式 | 计数器 CNT < CCR 时 | 计数器 CNT >= CCR 时 | 典型应用 |
|------|---------------------|---------------------|----------|
| PWM 模式 1 | OCxREF 输出高电平 | OCxREF 输出低电平 | **常用**，LED/电机控制 |
| PWM 模式 2 | OCxREF 输出低电平 | OCxREF 输出高电平 | 特殊极性需求 |

在 PWM 模式 1 下，向上计数过程中的波形生成逻辑：

```bob
  计数器 CNT 计数过程

  CNT    │        /|        /|        /|
         │       / |       / |       / |
         │      /  |      /  |      /  |
         │     /   |     /   |     /   |
  CCR    │----/    |    /    |    /    |
         │   /     |   /     |   /     |
         │  /      |  /      |  /      |
         │ /       | /       | /       |
  0      │/________|/________|/________|____
         └────────────────────────────────→ 时间
              ARR     ARR     ARR

  PWM 输出
         │ ┌──┐    ┌──┐    ┌──┐
         │ │  │    │  │    │  │
         │─┘  └────┘  └────┘  └────
         │ CCR 以上为高电平，CCR 以下为低电平
```

**图 1-3** PWM 模式 1 向上计数时的波形生成过程：CNT 从 0 递增到 ARR，当 CNT < CCR 时输出高电平，当 CNT >= CCR 时输出低电平。
<!-- fig:ch1-3 PWM 模式 1 向上计数时的波形生成过程：CNT 从 0 递增到 ARR，当 CNT < CCR 时输出高电平，当 CNT >= CCR 时输出低电平。 -->

#### 1.3.3 呼吸灯的实现原理

呼吸灯的核心思想是**周期性地改变 PWM 占空比**，使 LED 亮度呈三角波形渐变：

```bob
  亮度(%)
   100 │       /▲       /▲       /▲
       │      / \\     / \\     / \\
    50 │     /   \\   /   \\   /   \\
       │    /     \\ /     \\ /     \\
     0 │___/       V        V        V___→ 时间
       │┌──约 4s ──┐
```

**图 1-4** 呼吸灯亮度变化波形：占空比从 0% 线性增加到 100%，再从 100% 线性减小到 0%，形成周期性呼吸效果。
<!-- fig:ch1-4 呼吸灯亮度变化波形：占空比从 0% 线性增加到 100%，再从 100% 线性减小到 0%，形成周期性呼吸效果。 -->

实现方式是在主循环中以固定时间间隔（如 10ms）步进修改 CCR 值：

- **渐亮阶段**：CCR 从 0 递增到 ARR，步进值为 5
- **渐暗阶段**：CCR 从 ARR 递减到 0，步进值为 5
- **周期计算**：$T = 2 \times (ARR / step) \times \Delta t = 2 \times (1000 / 5) \times 10ms = 4s$

---

### 1.4 CubeMX 配置步骤

#### 1.4.1 工程创建与芯片选型

1. 打开 STM32CubeMX，点击 **New Project**
2. 在 MCU 选择器中搜索 `STM32F103C8`，选择 `STM32F103C8Tx`
3. 点击 **Start Project**

#### 1.4.2 时钟配置

1. 在 **Pinout & Configuration** 视图中，进入 **System Core → RCC**
2. **High Speed Clock (HSE)**：选择 `Crystal/Ceramic Resonator`
3. 切换到 **Clock Configuration** 视图：
   - HSE 输入频率：8 MHz
   - PLL 倍频：×9 → HCLK = 72 MHz
   - APB1 Prescaler：/2 → APB1 定时器时钟 = 72 MHz

![chapter13.4-1](/Users/william/Documents/WorkStation/embedded-systems-grad-course/docs/assets/images/chapter13/chapter13.4-1.png)

**图 1-5** CubeIDE时钟配置图

#### 1.4.3 TIM2 PWM 配置

**表 1-5** TIM2 CubeMX 配置参数
<!-- tab:ch1-5 TIM2 CubeMX 配置参数 -->

| 配置项 | 路径 | 参数值 | 说明 |
|--------|------|--------|------|
| 时钟源 | Timers → TIM2 → Clock Source | Internal Clock | 使用内部时钟 |
| 通道 1 | Timers → TIM2 → Channel 1 | PWM Generation CH1 | PWM 输出模式 |
| Prescaler | Parameter Settings | 71 | 72MHz / 72 = 1MHz |
| Counter Period | Parameter Settings | 999 | 1MHz / 1000 = 1kHz |
| Pulse | Parameter Settings | 0 | 初始占空比 0% |
| PWM Mode | Parameter Settings | PWM mode 1 | 标准 PWM 模式 |
| CH Polarity | Parameter Settings | High | 高电平有效 |

配置完成后，TIM2_CH1 自动映射到 **PA0** 引脚，该引脚被标记为绿色复用功能引脚。

![chapter13.4-2](/Users/william/Documents/WorkStation/embedded-systems-grad-course/docs/assets/images/chapter13/chapter13.4-2.png)

**图 1-6** CubeIDE TIM2 PWM 配置图

#### 1.4.4 GPIO 与调试接口

1. **PC13**：配置为 `GPIO_Output`（推挽输出），用作板载 LED 指示
2. **SYS → Debug**：选择 `Serial Wire`，启用 SWD 调试接口

#### 1.4.5 工程生成

1. 进入 **Project Manager** 视图
2. 设置工程名称：`PWM_Breathing_LED`
3. 工具链选择：`STM32CubeIDE`
4. 点击 **Generate Code**

![chapter13.4-3](/Users/william/Documents/WorkStation/embedded-systems-grad-course/docs/assets/images/chapter13/chapter13.4-3.png)

**图 1-7** CubeIDE 工程代码代码界面

**表 1-6** 生成的工程目录结构
<!-- tab:ch1-6 生成的工程目录结构 -->

| 目录/文件 | 说明 |
|-----------|------|
| `Core/Inc/` | 头文件目录（main.h、stm32f1xx_hal_conf.h 等） |
| `Core/Src/` | 源文件目录（main.c、stm32f1xx_hal_msp.c、stm32f1xx_it.c） |
| `Core/Startup/` | 启动文件（startup_stm32f103c8tx.s） |
| `Drivers/` | HAL 库与 CMSIS 驱动 |
| `.project` | CubeIDE 工程文件 |
| `.cproject` | C/C++ 工程配置 |

---

### 1.5 示例程序

#### 1.5.1 CubeMX 生成的初始化代码

CubeMX 自动生成的 TIM2 初始化函数位于 `Core/Src/main.c` 中：

```c
/**
 * @brief TIM2 初始化函数（CubeMX 自动生成）
 * @note  配置 TIM2_CH1 输出 1kHz PWM，PA0 引脚
 */
static void MX_TIM2_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;                    // 预分频：72MHz / 72 = 1MHz
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;  // 向上计数模式
  htim2.Init.Period = 999;                      // ARR：1MHz / 1000 = 1kHz
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_PWM_Init(&htim2);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;           // PWM 模式 1
  sConfigOC.Pulse = 0;                          // 初始占空比 0%
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;   // 高电平有效
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}
```

**代码关键点解析：**

**表 1-7** TIM2 初始化代码关键参数
<!-- tab:ch1-7 TIM2 初始化代码关键参数 -->

| 代码行 | 参数 | 计算过程 | 结果 |
|--------|------|----------|------|
| `Prescaler = 71` | 预分频值 | $f_{cnt} = 72MHz / (71+1)$ | 1 MHz |
| `Period = 999` | 自动重装值 | $f_{PWM} = 1MHz / (999+1)$ | 1 kHz |
| `Pulse = 0` | 初始 CCR 值 | $Duty = 0 / (999+1)$ | 0% |
| `OCMode = PWM1` | PWM 模式 | CNT < CCR 时输出高 | — |

#### 1.5.2 呼吸灯主循环代码

在 CubeMX 生成的 `USER CODE BEGIN` 和 `USER CODE END` 标记之间编写应用代码：

```c
/* USER CODE BEGIN 2 */
/* 启动 TIM2 通道 1 的 PWM 输出 */
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
uint16_t duty = 0;   /* 当前占空比值（0 ~ 999） */
int16_t  step = 5;   /* 每次步进量，正数渐亮，负数渐暗 */

while (1)
{
    /* 实时修改 CCR 值，改变占空比 */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);

    /* 三角波反弹逻辑：到达边界时反转方向 */
    duty += step;
    if (duty >= 1000) { duty = 999; step = -5; }
    if (duty <= 0)    { duty = 0;   step =  5; }

    HAL_Delay(10);  /* 10ms 步进间隔 */
}
/* USER CODE END WHILE */
```

**代码关键点解析：**

**表 1-8** 呼吸灯代码关键逻辑
<!-- tab:ch1-8 呼吸灯代码关键逻辑 -->

| 代码片段 | 说明 |
|---------|------|
| `HAL_TIM_PWM_Start` | 启动定时器 PWM 通道，硬件自动输出波形，CPU 无需干预 |
| `__HAL_TIM_SET_COMPARE` | 修改捕获/比较寄存器（CCR），实时改变占空比 |
| `duty += step` 反弹逻辑 | 三角波式渐变，step 正负决定渐亮或渐暗方向 |
| `HAL_Delay(10)` | 10ms 步进间隔，单程耗时 $1000/5 \times 10ms = 2s$ |
| `volatile` 语义 | 虽未显式声明 volatile，但 SET_COMPARE 通过指针访问外设寄存器 |

#### 1.5.3 完整 main.c 代码

以下是完整的 `main.c` 文件，包含 CubeMX 生成的框架代码和用户编写的应用代码：

```c
/**
 * @file    main.c
 * @brief   STM32F103C8T6 TIM2 PWM 呼吸灯
 * @note    CubeMX 生成框架 + 用户应用代码
 */

#include "main.h"
#include "tim.h"
#include "gpio.h"

TIM_HandleTypeDef htim2;

/**
 * @brief  系统时钟配置（CubeMX 自动生成）
 *         HSE 8MHz → PLL ×9 → HCLK 72MHz
 */
void SystemClock_Config(void);

int main(void)
{
  /* ===== 系统初始化（CubeMX 生成，勿修改）===== */
  HAL_Init();                // HAL 库初始化，配置 SysTick
  SystemClock_Config();      // 时钟树配置：HCLK = 72MHz
  MX_GPIO_Init();            // GPIO 初始化：PC13 板载 LED
  MX_TIM2_Init();            // TIM2 PWM 初始化：PA0, 1kHz

  /* ===== 用户代码区 ===== */

  /* USER CODE BEGIN 2 */
  /* 启动 TIM2 通道 1 的 PWM 输出 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  uint16_t duty = 0;   /* 当前占空比值（0 ~ 999） */
  int16_t  step = 5;   /* 每次步进量，正数渐亮，负数渐暗 */

  while (1)
  {
      /* 实时修改 CCR 值，改变占空比 */
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);

      /* 三角波反弹逻辑：到达边界时反转方向 */
      duty += step;
      if (duty >= 1000) { duty = 999; step = -5; }
      if (duty <= 0)    { duty = 0;   step =  5; }

      HAL_Delay(10);  /* 10ms 步进间隔，完整呼吸周期约 4 秒 */
  }
  /* USER CODE END WHILE */
}

/**
 * @brief  TIM2 初始化函数（CubeMX 自动生成）
 *         配置 TIM2_CH1 输出 1kHz PWM，PA0 引脚
 */
static void MX_TIM2_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_PWM_Init(&htim2);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}
```

---

### 1.6 PicSimlab 仿真验证

#### 1.6.1 仿真环境搭建

PicSimlab 是一款开源跨平台微控制器仿真器，通过 QEMU 后端仿真 STM32F103C8T6，可在无实物硬件的条件下验证固件功能。

**表 1-9** 仿真环境配置
<!-- tab:ch1-9 仿真环境配置 -->

| 项目 | 配置 |
|------|------|
| 仿真器 | PicSimlab（QEMU 后端） |
| 板卡 | stm32_blue_pill |
| IDE | STM32CubeIDE 1.14+ |
| 固件格式 | .bin（Debug 目录下编译产物） |

#### 1.6.2 仿真操作步骤

**步骤 1：编译固件**

1. 在 CubeIDE 中打开 `PWM_Breathing_LED` 工程
2. 选择 **Debug** 配置
3. 点击 **Build Project**（快捷键 Ctrl+B）
4. 编译成功后在 `Debug/` 目录下生成 `PWM_Breathing_LED.bin`

**步骤 2：加载固件到 PicSimlab**

1. 启动 PicSimlab
2. 选择板卡：`stm32_blue_pill`
3. 点击 **File → Load Hex/Bin**，选择 `Debug/PWM_Breathing_LED.bin`
4. 固件加载成功后，状态栏显示 `Firmware loaded`

**步骤 3：添加虚拟外设**

**表 1-10** PicSimlab 虚拟外设连接
<!-- tab:ch1-10 PicSimlab 虚拟外设连接 -->

| 虚拟组件 | 连接引脚 | 用途 | 属性配置 |
|---------|----------|------|----------|
| LED | PA0 | 观察呼吸效果 | 阳极接 PA0，阴极接 GND |
| Oscilloscope（探针 1） | PA0 | 观察 PWM 波形 | Time/Div: 500us |
| Oscilloscope（探针 2） | PC13 | 观察板载 LED | 可选 |

操作方式：
1. 在 PicSimlab 外设区域右键 → **Add Part → LED**
2. 右键 LED 组件 → **Properties** → 设置引脚为 `A0`（对应 PA0）
3. 同样添加 **Oscilloscope** 组件，探针 1 连接 `A0`

**步骤 4：运行仿真**

1. 点击 ▶ **Run** 按钮启动仿真
2. 观察 LED 亮度渐变效果和示波器波形

#### 1.6.3 预期现象

**LED 呼吸效果：**

- LED 从熄灭状态逐渐变亮，达到最亮后逐渐变暗，循环往复
- 完整呼吸周期约 4 秒（渐亮 2 秒 + 渐暗 2 秒）
- 亮度变化平滑，无明显闪烁

**示波器波形：**

```bob
  PicSimlab 示波器观察结果

  PA0 PWM 波形（占空比渐变过程）：

  时刻 T1（占空比 10%）
  PA0 │ ┌─┐       ┌─┐       ┌─┐
      │ │ │       │ │       │ │
      │─┘ └───────┘ └───────┘ └───

  时刻 T2（占空比 50%）
  PA0 │ ┌────┐     ┌────┐     ┌────┐
      │ │    │     │    │     │    │
      │─┘    └─────┘    └─────┘    └─

  时刻 T3（占空比 90%）
  PA0 │ ┌────────┐ ┌────────┐ ┌────────┐
      │ │        │ │        │ │        │
      │─┘        └─┘        └─┘        └─

  波形参数：
    频率 = 1 kHz（周期 = 1ms）
    占空比 = 0% → 100% → 0% 循环
    步进间隔 = 10ms
```

**图 1-8** PicSimlab 示波器观察到的 PWM 波形：频率恒定为 1kHz，占空比随时间从 0% 渐增到 100% 再渐减到 0%。
<!-- fig:ch1-8 PicSimlab 示波器观察到的 PWM 波形：频率恒定为 1kHz，占空比随时间从 0% 渐增到 100% 再渐减到 0%。 -->

#### 1.6.4 结果分析

**表 1-11** 仿真结果验证
<!-- tab:ch1-11 仿真结果验证 -->

| 验证项 | 理论值 | 仿真观测值 | 是否一致 |
|--------|--------|-----------|----------|
| PWM 频率 | 1 kHz | 示波器周期 1ms | 一致 |
| 占空比范围 | 0% ~ 100% | 示波器波形渐变 | 一致 |
| 呼吸周期 | 约 4 秒 | LED 亮度渐变周期 | 一致 |
| 亮度平滑度 | 0.5% 步进 | 无明显阶梯感 | 一致 |

**频率验证计算：**

$$
f_{PWM} = \frac{f_{clk}}{(PSC+1) \times (ARR+1)} = \frac{72MHz}{72 \times 1000} = 1kHz
$$

**占空比精度：**

$$
Res = \frac{1}{ARR+1} \times 100\% = \frac{1}{1000} \times 100\% = 0.1\%
$$

> **注意**：PicSimlab 使用 QEMU 后端仿真，其时序精度与真实硬件存在差异。仿真环境无法完全模拟电气噪声、上电时序抖动等物理现象，实物验证仍不可或缺。

![chapter13.4-4](/Users/william/Documents/WorkStation/embedded-systems-grad-course/docs/assets/images/chapter13/chapter13.4-4.png)

**图 1-9** PicSimlab 仿真结果 （LED全灭)

![chapter13.4-4](/Users/william/Documents/WorkStation/embedded-systems-grad-course/docs/assets/images/chapter13/chapter13.4-5.png)

**图 1-10** PicSimlab 仿真结果 （LED 50% 亮度）

![chapter13.4-6](/Users/william/Documents/WorkStation/embedded-systems-grad-course/docs/assets/images/chapter13/chapter13.4-6.png)

**图 1-11** PicSimlab 仿真结果 （LED 100% 亮度）

---

### 1.7 拓展与思考

#### 1.7.1 非线性亮度映射

人眼对亮度的感知是非线性的（符合韦伯-费希纳定律），线性变化的 PWM 占空比在人眼看来并不均匀。可以通过伽马校正或正弦映射改善视觉效果：

**表 1-12** 亮度映射方式对比
<!-- tab:ch1-12 亮度映射方式对比 -->

| 映射方式 | 公式 | 效果 | 适用场景 |
|---------|------|------|----------|
| 线性映射 | $CCR = duty$ | 步进均匀，但视觉不均匀 | 简单控制 |
| 伽马校正 | $CCR = (duty/1000)^{2.2} \times 1000$ | 暗区步进小，亮区步进大 | LED 调光 |
| 正弦映射 | $CCR = (1 - \cos(\pi \times duty/1000)) / 2 \times 1000$ | 感知均匀 | 呼吸灯 |

#### 1.7.2 多 LED 呼吸灯

扩展为多路 PWM 输出，驱动多个 LED 实现交错呼吸效果。可使用 TIM2 的 CH1~CH4 同时输出 4 路 PWM，各路设置不同的相位偏移。

#### 1.7.3 基于中断的占空比更新

将占空比更新逻辑从主循环移至定时器中断回调中，释放 CPU 资源：

```c
/* 在 TIM 更新中断回调中更新占空比 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint16_t duty = 0;
    static int16_t  step = 5;

    if (htim->Instance == TIM2)
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
        duty += step;
        if (duty >= 1000) { duty = 999; step = -5; }
        if (duty <= 0)    { duty = 0;   step =  5; }
    }
}
```

此方式将 CPU 占用降至最低，主循环可处理其他任务。

---

### 1.8 本章小结

本章以 STM32F103C8T6 为平台，系统讲解了定时器 PWM 呼吸灯的完整开发流程。

**表 1-13** 本章核心知识点汇总
<!-- tab:ch1-13 本章核心知识点汇总 -->

| 知识点 | 内容 | 关键参数/函数 |
|--------|------|--------------|
| PWM 原理 | 通过改变高电平占比等效调节平均电压 | 占空比 = CCR / (ARR+1) |
| 频率计算 | $f = f_{clk} / ((PSC+1)(ARR+1))$ | PSC=71, ARR=999 → 1kHz |
| CubeMX 配置 | TIM2 Channel 1 PWM Generation | PWM mode 1, PA0 |
| HAL 库启动 | `HAL_TIM_PWM_Start` | 启动后硬件自动输出波形 |
| 占空比修改 | `__HAL_TIM_SET_COMPARE` | 实时修改 CCR，无需重启 |
| 呼吸灯算法 | 三角波反弹 + 定时步进 | step=5, delay=10ms |
| 仿真验证 | PicSimlab 示波器观察波形 | 频率 1kHz，占空比渐变 |

**STM32 外设编程三步法（PWM 版）：**

```bob
  ① 初始化           ② 启动                  ③ 使用
  MX_TIM2_Init( )    HAL_TIM_PWM_Start( )    __HAL_TIM_SET_COMPARE( )
       |                      |                         |
       v                      v                         v
  CubeMX 生成             调用一次               主循环或中断中调用
```

**图 1-12** STM32 PWM 外设编程三步法：初始化 → 启动 → 使用。
<!-- fig:ch1-12 STM32 PWM 外设编程三步法：初始化 → 启动 → 使用。 -->

> **拓展阅读**：定时器还可用于输入捕获（测量外部信号频率）、编码器接口（读取旋转编码器）、主从同步（多定时器联动）等高级应用，详见第 5 章。

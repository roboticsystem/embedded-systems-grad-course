---
number headings: first-level 2, start-at 12
---

## 12 第 12 章 嵌入式系统综合设计

> 前面各章分别介绍了外设驱动、传感器、执行器、通信和控制。本章将从系统层面讨论嵌入式产品的设计方法论，包括低功耗设计、固件架构、可靠性保障和调试技术，帮助学生从"写一个外设驱动"提升到"设计一个完整的嵌入式产品"。

### 12.1 本章知识导图

```plantuml
@startmindmap
skinparam mindmapNodeBackgroundColor<<root>>    #1565C0
skinparam mindmapNodeFontColor<<root>>          white
skinparam mindmapNodeBackgroundColor<<l1>>      #1976D2
skinparam mindmapNodeFontColor<<l1>>            white
skinparam ArrowColor                            #90CAF9
skinparam mindmapNodeBorderColor                #90CAF9

* 第12章 嵌入式系统综合设计
** 固件架构
*** 裸机轮询架构
*** 前后台架构
*** RTOS 架构
*** 分层/模块化设计
** 低功耗设计
*** STM32 低功耗模式
*** 唤醒源配置
*** 功耗预算计算
** 可靠性保障
*** 看门狗（IWDG/WWDG）
*** 断言与错误处理
*** Flash 参数存储
** 调试与测试
*** SWD/JTAG 调试
*** printf 重定向
*** 逻辑分析仪
** 案例：智能温室控制器
@endmindmap
```

**图 12-1** 本章知识导图：嵌入式系统综合设计的关键要素。
<!-- fig:ch12-1 本章知识导图：嵌入式系统综合设计的关键要素。 -->

### 12.2 固件架构设计

#### 12.2.1 三种典型架构

**表 12-1** 三种固件架构对比
<!-- tab:ch12-1 三种固件架构对比 -->

| 架构 | 结构 | 优点 | 缺点 | 适用场景 |
|------|------|------|------|---------|
| 裸机轮询 | `while(1)` 顺序执行 | 简单直观 | 无法响应实时事件 | LED 闪烁等极简应用 |
| 前后台 | 中断（前台）+ `while(1)`（后台） | 实时性好 | 中断中不宜执行耗时操作 | 大多数中小型项目 |
| RTOS | 多任务调度 | 模块化、可扩展 | 占用 RAM、学习成本高 | 复杂多任务系统 |

#### 12.2.2 前后台架构实践

前后台架构是嵌入式开发最常用的模式——中断中置标志位/存数据，主循环中处理逻辑：

```c
/* 全局标志 */
volatile uint8_t flag_uart_rx = 0;
volatile uint8_t flag_tim_10ms = 0;
volatile uint8_t flag_adc_done = 0;

/* 中断：置标志 + 快速存数据 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
        flag_tim_10ms = 1;
}

/* 主循环：依次处理各事件 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    Periph_Init();

    while (1) {
        if (flag_tim_10ms) {
            flag_tim_10ms = 0;
            PID_Control_Task();    /* 10ms 周期控制 */
        }
        if (flag_uart_rx) {
            flag_uart_rx = 0;
            Command_Parse_Task();  /* 解析串口指令 */
        }
        if (flag_adc_done) {
            flag_adc_done = 0;
            Sensor_Process_Task(); /* 传感器数据处理 */
        }
        Display_Update_Task();     /* 低优先级刷新显示 */
    }
}
```

#### 12.2.3 分层模块化设计

```bob
  ┌─────────────────────────────────────────────┐
  │               应用层 (Application)           │
  │  main.c, task_xxx.c                         │
  ├─────────────────────────────────────────────┤
  │               服务层 (Service)               │
  │  pid.c, protocol.c, mqtt.c                  │
  ├─────────────────────────────────────────────┤
  │               驱动层 (Driver)                │
  │  motor.c, dht11.c, oled.c, esp8266.c       │
  ├─────────────────────────────────────────────┤
  │               硬件抽象层 (HAL)               │
  │  stm32f1xx_hal_xxx.c                        │
  ├─────────────────────────────────────────────┤
  │               硬件 (Hardware)                │
  │  STM32F103C8T6 + 外围电路                    │
  └─────────────────────────────────────────────┘
```

**图 12-2** 分层固件架构：上层调用下层接口，禁止跨层或反向调用。
<!-- fig:ch12-2 分层固件架构：上层调用下层接口，禁止跨层或反向调用。 -->

---

### 12.3 低功耗设计

电池供电的农业传感器节点需要极低功耗以延长工作时间。STM32F103 提供三种低功耗模式：

**表 12-2** STM32F103 低功耗模式对比
<!-- tab:ch12-2 STM32F103 低功耗模式对比 -->

| 模式 | 内核 | 外设 | RAM | 唤醒源 | 典型电流 |
|------|:----:|:----:|:---:|--------|:-------:|
| Sleep | 停止 | 运行 | 保持 | 任意中断 | ~1.5 mA |
| Stop | 停止 | 停止 | 保持 | EXTI/RTC | ~20 μA |
| Standby | 停止 | 停止 | 丢失 | WKUP/RTC/NRST | ~2 μA |

#### 12.3.1 Stop 模式实现

```c
/* 进入 Stop 模式，RTC 闹钟唤醒 */
void EnterStopMode(uint32_t wakeup_sec)
{
    /* 配置 RTC 闹钟 */
    HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);

    /* 进入 Stop 模式 */
    HAL_SuspendTick();
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    /* 唤醒后恢复时钟 */
    SystemClock_Config();
    HAL_ResumeTick();
}

/* 典型工作模式：采集 → 发送 → 休眠 */
while (1) {
    Sensor_ReadAll();          /* 采集传感器数据 */
    MQTT_PublishData();        /* 上报数据 */
    EnterStopMode(300);        /* 休眠 5 分钟 */
}
```

#### 12.3.2 功耗预算计算

以 3.7V/2000mAh 锂电池供电为例：

**表 12-3** 
<!-- tab:ch12-3  -->

| 阶段 | 时间 | 电流 | 能量消耗 |
|------|:----:|:----:|:-------:|
| 采集+发送 | 5s | 50 mA | 0.069 mAh |
| 休眠 | 295s | 20 μA | 0.0016 mAh |
| **一个周期（5min）** | **300s** | — | **0.071 mAh** |

$$寿命 = \frac{2000 \text{ mAh}}{0.071 \times 12} = 2347 \text{ 小时} \approx 98 \text{ 天}$$

---

### 12.4 可靠性保障

在工业级嵌入式产品设计中，可靠性不仅意味着代码没有 Bug，更意味着系统在受到电磁干扰（EMI）、电源波动或软件偶发异常时能够自动恢复。本节重点介绍独立看门狗和非易失性存储两个核心模块。

#### 12.4.1 独立看门狗（IWDG）

IWDG 是一个独立的硬件定时器，它使用专用的低速内部时钟（LSI，约 40kHz）。由于 LSI 独立于主系统时钟（HSE/HSI），即使内核因时钟配置错误卡死，IWDG 依然能正常工作。

**1. 配置原理**
超时时间 $T_{out}$ 的计算公式为：
$T_{out} = \frac{Prescaler \times Reload}{f_{LSI}}$
例如：预分频设为 64，重装载值设为 625，则 $T_{out} = \frac{64 \times 625}{40000} = 1.0$ 秒。

**2. C 代码实现**
```c
IWDG_HandleTypeDef hiwdg;

/* IWDG 初始化：配置为 1s 超时 */
void WDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64; // 预分频器 64
    hiwdg.Init.Reload = 625;                  // 重装载值 625 (1秒)
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        /* 初始化失败通常意味着硬件异常 */
        Error_Handler();
    }
}

/* 喂狗逻辑：必须在 1s 内调用一次 */
void WDG_Refresh(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}
```

#### 12.4.2 Flash 参数存储

STM32 的内部 Flash 除了存放程序，常被用于模拟 EEPROM 存储配置参数（如 PID 调优值、设备 ID）。

**1. 操作规则**
*   **先擦后写**：Flash 只能将位从 1 写为 0。因此，在写入新数据前必须按“页”擦除（将整页设为 0xFFFF）。
*   **解锁与锁定**：操作 Flash 前必须解锁，操作完成后立即锁定以防程序跑飞时误写。

**2. C 代码实现**
```c
#define PARAM_PAGE_ADDR  0x0800FC00  /* Flash 最后一页起始地址 (Page 63) */

/**
 * @brief 保存参数到 Flash
 * @param data 指向数据的指针, len 字节长度 (必须为2的倍数)
 */
void Param_Save(const uint8_t *data, uint16_t len)
{
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;

    HAL_FLASH_Unlock(); // 1. 解锁

    /* 2. 配置擦除参数：擦除单页 */
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = PARAM_PAGE_ADDR;
    EraseInitStruct.NbPages     = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
        Error_Handler();
    }

    /* 3. 循环写入半字 (16-bit) */
    for (uint16_t i = 0; i < len; i += 2) {
        uint16_t val = data[i] | (data[i + 1] << 8);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 
                             PARAM_PAGE_ADDR + i, val) != HAL_OK) {
            Error_Handler();
        }
    }

    HAL_FLASH_Lock(); // 4. 锁定
}

/**
 * @brief 从 Flash 读取参数
 */
void Param_Load(uint8_t *data, uint16_t len)
{
    /* Flash 可以像内存一样通过指针直接读取 */
    memcpy(data, (const void *)PARAM_PAGE_ADDR, len);
}
```

#### 12.4.3 错误处理框架

`Error_Handler` 是系统的最后一道防线。当检测到不可恢复的错误（如 Flash 校验失败、传感器断路）时，系统应进入安全状态并等待重启。

```c
/**
 * @brief 系统错误处理函数
 */
void Error_Handler(void)
{
    /* 1. 禁用全局中断，防止干扰 */
    __disable_irq();

    /* 2. 进入死循环，并以特定频率闪烁 LED 报警 */
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // 假设 LED 在 PC13
        for (volatile uint32_t i = 0; i < 500000; i++); 

        /* 注意：此处故意不喂狗！
           当 IWDG 计时达到 1s 后，硬件将自动产生复位信号，
           从而使系统尝试重新初始化，实现故障自恢复。 */
    }
}
```


#### 12.4.4 综合代码示例

本小节展示如何在一个典型的“传感器采集”应用中整合看门狗和 Flash 存储功能。系统启动时会从 Flash 加载配置，在运行过程中周期性喂狗，并在检测到异常时通过停止喂狗实现自恢复。

**1. 程序逻辑流程**

```plantuml
@startuml
skinparam ActivityBackgroundColor #E3F2FD
skinparam ActivityBorderColor #1565C0

start
:系统初始化 (HAL/时钟/GPIO);
:从 Flash 加载运行参数;
if (参数合法?) then (否)
  :使用默认参数;
endif
:启动 IWDG (1s 超时);

repeat
  :读取传感器数据;
  :处理业务逻辑;
  if (发生严重错误?) then (是)
    :进入 Error_Handler;
    note right: 停止喂狗，等待重启
  endif
  :执行 IWDG 喂狗;
  :进入低功耗延时;
repeat while (true)
@enduml
```

**2. 完整 C 代码实现 (核心逻辑)**

```c
/* 包含必要的头文件 */
#include "main.h"
#include <string.h>

/* 配置参数结构体 */
typedef struct {
    uint16_t device_id;
    float threshold;
    uint32_t magic_num; // 用于校验参数是否有效
} Config_t;

#define CONFIG_MAGIC  0x5A5A1234
Config_t g_config;
IWDG_HandleTypeDef hiwdg;

/* 函数声明 */
void SystemClock_Config(void);
void WDG_Init(void);
void Param_Load(Config_t *conf);
void Param_Save(Config_t *conf);

int main(void)
{
    /* 1. 硬件基础初始化 */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    
    /* 2. 恢复参数 */
    Param_Load(&g_config);
    if (g_config.magic_num != CONFIG_MAGIC) {
        // 如果 Flash 中没有有效参数，初始化为默认值
        g_config.device_id = 0x0001;
        g_config.threshold = 25.5f;
        g_config.magic_num = CONFIG_MAGIC;
        Param_Save(&g_config);
    }

    /* 3. 启动看门狗 */
    WDG_Init();

    /* 4. 主循环 */
    while (1)
    {
        // 模拟传感器采集
        float temperature = Read_Sensor();
        
        // 异常判断示例
        if (temperature > 100.0f || temperature < -40.0f) {
            Error_Handler(); // 传感器异常，进入错误处理并等待重启
        }

        // 正常业务逻辑...
        Process_Data(temperature);

        // 5. 及时喂狗
        HAL_IWDG_Refresh(&hiwdg);

        HAL_Delay(100); // 循环周期
    }
}

/* --- 可靠性模块的具体实现 --- */

void WDG_Init(void) {
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Reload = 625; // 1秒超时
    HAL_IWDG_Init(&hiwdg);
}

void Param_Save(Config_t *conf) {
    FLASH_EraseInitTypeDef erase;
    uint32_t error;
    
    HAL_FLASH_Unlock();
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = 0x0800FC00; // Page 63
    erase.NbPages = 1;
    
    if (HAL_FLASHEx_Erase(&erase, &error) == HAL_OK) {
        uint16_t *pData = (uint16_t *)conf;
        for (uint16_t i = 0; i < sizeof(Config_t); i += 2) {
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, 0x0800FC00 + i, *pData++);
        }
    }
    HAL_FLASH_Lock();
}

void Param_Load(Config_t *conf) {
    memcpy(conf, (void*)0x0800FC00, sizeof(Config_t));
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
        // 闪烁 LED 报错，不喂狗，等待 IWDG 复位
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}
```

---

### 12.5 调试技术

#### 12.5.1 printf 重定向到串口

```c
/* 重定向 printf 到 USART1（需勾选 Use MicroLIB） */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 10);
    return ch;
}

/* 使用示例 */
printf("RPM=%.1f, PID_out=%.1f\r\n", rpm, pid_output);
```

#### 12.5.2 关键调试手段

**表 12-4** 嵌入式系统调试手段
<!-- tab:ch12-4 嵌入式系统调试手段 -->

| 手段 | 工具 | 适用场景 |
|------|------|---------|
| 串口打印 | USART + 串口助手 | 运行时变量监控 |
| SWD 在线调试 | ST-Link + CubeIDE | 断点、单步、变量查看 |
| 逻辑分析仪 | Saleae / PulseView | 时序分析（I2C/SPI/UART 波形） |
| LED 指示 | 板载 LED | 快速定位程序是否运行到某阶段 |
| PicSimlab 仿真 | PicSimlab（第 6 章） | 无硬件条件下的全功能调试 |

---

### 12.6 综合案例：智能温室控制器

整合本书前 11 章的知识，设计一个完整的智能温室控制器系统：

**表 12-5** 系统功能需求
<!-- tab:ch12-5 系统功能需求 -->

| 模块 | 功能 | 涉及章节 |
|------|------|---------|
| 传感器采集 | DHT11 温湿度 + 光照 ADC + 土壤湿度 ADC | 第 7 章 |
| 本地显示 | OLED 显示当前参数和系统状态 | 第 8 章 |
| 执行控制 | 直流电机驱动风扇、步进电机控制遮阳帘 | 第 9 章 |
| 自动调温 | PID 控制风扇速度，维持目标温度 | 第 10 章 |
| 远程通信 | CAN 总线多节点 + Wi-Fi/MQTT 上云 | 第 11 章 |
| 低功耗 | 夜间进入 Stop 模式，RTC 定时唤醒 | 第 12 章 |
| 可靠性 | 看门狗、参数 Flash 存储、异常自恢复 | 第 12 章 |

**软件流程：**

```plantuml
@startuml
skinparam ActivityBackgroundColor #E3F2FD
skinparam ActivityBorderColor #1565C0

start
:系统初始化;
note right: HAL/时钟/外设/恢复参数
:启动看门狗;

repeat
  :喂狗;
  :读取传感器;
  :PID 计算;
  :驱动执行器;
  :更新 OLED 显示;
  if (通信周期到?) then (是)
    :上报 MQTT;
    :检查下行指令;
  endif
  if (夜间模式?) then (是)
    :关闭执行器;
    :进入 Stop 模式;
    note right: RTC 30min 唤醒
  endif
repeat while (true)
@enduml
```

**图 12-3** 智能温室控制器主程序流程。
<!-- fig:ch12-3 智能温室控制器主程序流程。 -->

---

### 12.7 本章小结

- **固件架构**：前后台架构适合大多数嵌入式项目，代码应分层组织（应用→服务→驱动→HAL）
- **低功耗设计**：Stop 模式 + RTC 唤醒可将功耗降至 μA 级，电池供电可达数月
- **可靠性**：看门狗防死机、Flash 存储防断电丢参数
- **调试**：printf 重定向 + SWD 调试 + PicSimlab 仿真是最实用的三板斧
- **系统集成**：综合设计需要从需求分析→架构设计→模块实现→联调测试系统化推进

---

### 12.8 习题

1. 比较裸机轮询、前后台和 RTOS 三种架构的优缺点，你的毕业设计项目适合哪种架构？
2. 计算：若传感器节点每 10 分钟采集并发送一次数据，采集发送阶段 3 秒、50mA，休眠 20μA。使用 18650 电池（3400mAh）能工作多久？
3. 独立看门狗（IWDG）和窗口看门狗（WWDG）的区别是什么？各适用于什么场景？
4. 为什么 STM32 的 Flash 编程必须先擦除后写入？最小擦除单位是什么？
5. 画出你设计的一个嵌入式产品（如智能鱼缸、自动浇花器等）的系统框图，标明传感器、执行器、通信方式和电源方案。
## 2 嵌入式分层架构模式：HAL 接口定义与 LED 驱动封装

> 本文以 STM32F103C8T6（Blue Pill）为硬件平台，系统讲解嵌入式分层架构的设计思想与工程实践。通过定义统一的 HAL（硬件抽象层）接口，实现 LED 驱动的跨平台封装，并在 CubeMX + CubeIDE 工程中完成编码与编译验证，最终通过 PicSimLab 仿真环境演示 LED 闪烁效果。

### 2.1 学习目标

完成本章学习后，读者应能够：

1. 理解嵌入式分层架构的必要性与各层职责划分
2. 掌握 HAL 接口定义的方法与 C 语言实现技巧
3. 能够基于 HAL 接口封装设备驱动（以 LED 为例）
4. 使用 CubeMX + CubeIDE 创建分层架构工程
5. 在 PicSimLab 仿真环境中验证分层架构驱动的功能正确性

---

### 2.2 分层架构概述

#### 2.2.1 为什么需要分层架构

嵌入式项目代码规模扩大后，最常见的问题是"**牵一发而动全身**"——更换芯片时应用逻辑全部需要修改，修改外设驱动时业务代码也随之出错。分层架构通过**明确的职责边界**解决这一问题。

**表 2-1** 分层架构解决的核心问题

| 问题 | 无分层时的表现 | 分层后的改善 |
|------|--------------|-------------|
| 芯片移植 | 所有代码重写 | 仅替换 HAL 实现层 |
| 驱动更换 | 业务逻辑连带修改 | 驱动层独立替换 |
| 单元测试 | 必须依赖硬件 | 可 Mock HAL 层进行测试 |
| 团队协作 | 代码耦合严重 | 各层独立开发、接口约定 |

#### 2.2.2 分层架构模型

嵌入式系统的经典分层架构由五层组成，自上而下依次为应用层、服务层、驱动层、HAL 层和硬件层：

```bob
  嵌入式系统分层架构

  ┌─────────────────────────────────────────────────┐
  │              应用层  (Application )             │
  │           业务逻辑、状态机、任务调度            │
  ├─────────────────────────────────────────────────┤
  │              服务层  (Service )                 │
  │        传感器融合、协议解析、数据处理           │
  ├─────────────────────────────────────────────────┤
  │              驱动层  (Driver )                  │
  │        LED 驱动、UART 驱动、OLED 驱动           │
  ├─────────────────────────────────────────────────┤
  │          硬件抽象层  (HAL / BSP )               │
  │        hal_gpio_write / hal_gpio_read           │
  ├─────────────────────────────────────────────────┤
  │              硬件  (Hardware )                  │
  │           MCU GPIO、传感器、执行器              │
  └─────────────────────────────────────────────────┘

  调用方向：上层 → 下层（单向依赖）
```

**图 2-1** 嵌入式系统分层架构模型：五层结构自上而下单向调用，每层只依赖其直接下层。

**各层职责对比：**

**表 2-2** 分层架构各层职责

| 层次 | 职责 | 典型内容 | 允许依赖 |
|------|------|---------|---------|
| 应用层 | 实现系统功能与业务逻辑 | 任务调度、状态机、UI | 服务层 |
| 服务层 | 对驱动进行组合与封装 | 温度计算、滤波算法 | 驱动层 |
| 驱动层 | 操作具体外设 | LED 驱动、OLED 驱动 | HAL 层 |
| HAL 层 | 屏蔽芯片差异 | hal_gpio_write/read | 硬件 |
| 硬件层 | 物理电路 | MCU、LED、电阻 | 无 |

#### 2.2.3 分层架构的核心原则

分层架构遵循以下设计原则：

**表 2-3** 分层架构核心设计原则

| 原则 | 含义 | 工程体现 |
|------|------|---------|
| 依赖倒置 | 上层不依赖下层具体实现，依赖接口 | 应用层调用 `led_on()` 而非 `HAL_GPIO_WritePin()` |
| 单一职责 | 每层只做一件事 | HAL 层只负责硬件操作，不含业务逻辑 |
| 接口隔离 | 上层只看到必要的接口 | 驱动层暴露 `led_on/off/toggle`，隐藏内部实现 |
| 开闭原则 | 对扩展开放，对修改关闭 | 新增平台只需实现 HAL 接口，不修改驱动层 |

**层间调用关系图：**

```bob
  层间调用关系（以 LED 控制为例）

  ┌──────────────┐
  │   应用层     │
  │   main( )    │
  └──────┬───────┘
         │ 调用 led_on( )
         v
  ┌──────────────┐
  │   驱动层     │
  │  led_driver  │
  └──────┬───────┘
         │ 调用 hal_gpio_write( )
         v
  ┌──────────────┐
  │   HAL 层     │
  │  hal_gpio    │
  └──────┬───────┘
         │ 调用 HAL_GPIO_WritePin( )
         v
  ┌──────────────┐
  │   硬件层     │
  │  STM32 GPIO  │
  └──────────────┘
```

**图 2-2** 层间调用关系：应用层通过驱动层间接操作硬件，每层只调用其直接下层的接口。

---

### 2.3 HAL 接口定义

#### 2.3.1 HAL 接口设计目标

HAL（Hardware Abstraction Layer）接口的核心目标是**屏蔽芯片差异**，使上层代码在移植时无需修改。以 GPIO 操作为例，不同芯片厂商的 API 差异如下：

**表 2-4** 不同芯片厂商 GPIO API 对比

| 厂商 | 芯片系列 | GPIO 写操作 API | 头文件 |
|------|---------|----------------|--------|
| ST | STM32F1 | `HAL_GPIO_WritePin(GPIOx, Pin, State)` | `stm32f1xx_hal_gpio.h` |
| ST | STM32F4 | `HAL_GPIO_WritePin(GPIOx, Pin, State)` | `stm32f4xx_hal_gpio.h` |
| NXP | LPC | `GPIO_PinWrite(port, pin, value)` | `fsl_gpio.h` |
| Espressif | ESP32 | `gpio_set_level(gpio_num, level)` | `driver/gpio.h` |

HAL 接口定义统一的抽象 API，将上述差异封装在实现层中。

#### 2.3.2 HAL GPIO 接口定义

以下是平台无关的 HAL GPIO 接口头文件，仅包含类型定义和函数声明，不依赖任何具体芯片头文件：

```c
/**
 * @file    hal_gpio.h
 * @brief   硬件抽象层 — GPIO 接口定义
 * @note    平台无关，不包含任何芯片特定头文件
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

/**
 * @brief GPIO 引脚状态枚举
 */
typedef enum {
    GPIO_PIN_RESET = 0,   /**< 引脚低电平 */
    GPIO_PIN_SET   = 1    /**< 引脚高电平 */
} GPIO_PinState;

/**
 * @brief  设置 GPIO 引脚输出电平
 * @param  port  端口索引（0=GPIOA, 1=GPIOB, 2=GPIOC, ...）
 * @param  pin   引脚编号（0~15）
 * @param  state 目标电平（GPIO_PIN_RESET 或 GPIO_PIN_SET）
 */
void hal_gpio_write(uint8_t port, uint8_t pin, GPIO_PinState state);

/**
 * @brief  翻转 GPIO 引脚电平
 * @param  port  端口索引
 * @param  pin   引脚编号
 */
void hal_gpio_toggle(uint8_t port, uint8_t pin);

/**
 * @brief  读取 GPIO 引脚当前电平
 * @param  port  端口索引
 * @param  pin   引脚编号
 * @return 引脚电平状态
 */
GPIO_PinState hal_gpio_read(uint8_t port, uint8_t pin);

#endif /* HAL_GPIO_H */
```

**接口设计要点：**

**表 2-5** HAL GPIO 接口设计要点

| 设计要点 | 说明 | 好处 |
|---------|------|------|
| 使用 `uint8_t` 索引端口 | 不依赖 `GPIO_TypeDef*` | 接口头文件不包含芯片头文件 |
| 使用枚举表示电平 | 类型安全，语义清晰 | 避免魔术数字 0/1 |
| 纯接口声明 | 仅函数原型，无实现 | 实现文件可按平台替换 |
| 统一命名规范 | `hal_gpio_` 前缀 | 避免命名冲突，便于识别 |

#### 2.3.3 HAL GPIO 接口实现（STM32 平台）

以下是针对 STM32F1 平台的 HAL GPIO 接口实现文件。该文件包含芯片特定的头文件和映射逻辑：

```c
/**
 * @file    hal_gpio_stm32.c
 * @brief   HAL GPIO 接口的 STM32 平台实现
 * @note    仅此文件依赖 STM32 HAL 库
 */

#include "hal_gpio.h"
#include "stm32f1xx_hal.h"

/**
 * @brief 端口索引到 GPIO_TypeDef 指针的映射表
 * @note  port=0 → GPIOA, port=1 → GPIOB, port=2 → GPIOC
 */
static GPIO_TypeDef* port_map[] = {
    GPIOA, GPIOB, GPIOC
};

#define PORT_MAP_SIZE  (sizeof(port_map) / sizeof(port_map[0]))

void hal_gpio_write(uint8_t port, uint8_t pin, GPIO_PinState state)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        HAL_GPIO_WritePin(port_map[port], (1U << pin), state);
    }
}

void hal_gpio_toggle(uint8_t port, uint8_t pin)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        HAL_GPIO_TogglePin(port_map[port], (1U << pin));
    }
}

GPIO_PinState hal_gpio_read(uint8_t port, uint8_t pin)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        return HAL_GPIO_ReadPin(port_map[port], (1U << pin));
    }
    return GPIO_PIN_RESET;
}
```

**移植到其他平台时**，只需替换 `hal_gpio_stm32.c` 为对应平台的实现文件（如 `hal_gpio_nxp.c`、`hal_gpio_esp32.c`），上层代码无需任何修改。

**跨平台移植流程：**

```bob
  跨平台移植流程

  原平台（STM32）                    新平台（NXP LPC）
  ┌─────────────────┐               ┌─────────────────┐
  │   应用层        │               │   应用层        │
  │   (不修改)      │               │   (不修改)      │
  ├─────────────────┤               ├─────────────────┤
  │   驱动层        │               │   驱动层        │
  │   (不修改)      │               │   (不修改)      │
  ├─────────────────┤               ├─────────────────┤
  │   hal_gpio.h    │───────────────│   hal_gpio.h    │
  │   (不修改)      │   复制        │   (不修改)      │
  ├─────────────────┤               ├─────────────────┤
  │ hal_gpio_stm32.c│─-─替换为─-─→  │ hal_gpio_nxp.c  │
  │    (删除)       │               │   (新实现)      │
  └─────────────────┘               └─────────────────┘
```

**图 2-3** 跨平台移植流程：仅需替换 HAL 实现文件，应用层和驱动层代码零修改。

---

### 2.4 LED 驱动封装

#### 2.4.1 驱动层设计思想

驱动层基于 HAL 接口，进一步封装**语义化操作**。应用层不需要知道"LED 连接在 PC13 引脚，低电平点亮"这些硬件细节，只需要调用 `led_on()`、`led_off()` 即可。

**表 2-6** 驱动层与 HAL 层的职责对比

| 对比项 | HAL 层 | 驱动层 |
|--------|--------|--------|
| 操作对象 | 通用 GPIO 引脚 | 特定外设（LED） |
| 接口语义 | `hal_gpio_write(port, pin, state)` | `led_on()` |
| 硬件细节 | 暴露端口、引脚编号 | 隐藏在驱动内部 |
| 可移植性 | 平台相关实现 | 平台无关（依赖 HAL 接口） |
| 使用者 | 驱动开发者 | 应用开发者 |

#### 2.4.2 LED 驱动头文件

```c
/**
 * @file    led_driver.h
 * @brief   LED 驱动层 — 基于 HAL GPIO 接口封装
 */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

/**
 * @brief LED 硬件连接配置
 * @note  修改此处即可适配不同硬件，无需修改驱动逻辑
 */
#define LED_PORT    2       /**< GPIOC（port_map 索引 2） */
#define LED_PIN     13      /**< PC13 引脚 */

/**
 * @brief  点亮 LED
 * @note   PC13 板载 LED 低电平点亮（低电平有效）
 */
void led_on(void);

/**
 * @brief  熄灭 LED
 */
void led_off(void);

/**
 * @brief  翻转 LED 状态
 */
void led_toggle(void);

#endif /* LED_DRIVER_H */
```

#### 2.4.3 LED 驱动实现文件

```c
/**
 * @file    led_driver.c
 * @brief   LED 驱动层实现
 */

#include "led_driver.h"
#include "hal_gpio.h"

void led_on(void)
{
    /* PC13 LED 低电平点亮 */
    hal_gpio_write(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

void led_off(void)
{
    /* PC13 LED 高电平熄灭 */
    hal_gpio_write(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void led_toggle(void)
{
    hal_gpio_toggle(LED_PORT, LED_PIN);
}
```

#### 2.4.4 应用层调用示例

应用层只需包含 `led_driver.h`，无需了解底层硬件细节：

```c
/**
 * @file    main.c
 * @brief   应用层 — LED 闪烁示例
 * @note    基于分层架构，应用层不直接操作硬件寄存器
 */

#include "main.h"
#include "gpio.h"
#include "led_driver.h"

int main(void)
{
    /* ===== 系统初始化（CubeMX 生成）===== */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* ===== 应用逻辑 ===== */
    while (1)
    {
        led_toggle();       /* 翻转 LED 状态 */
        HAL_Delay(500);     /* 延时 500ms */
    }
}
```

**代码调用链分析：**

**表 2-7** LED 闪烁调用链

| 调用层次 | 函数 | 文件 | 说明 |
|---------|------|------|------|
| 应用层 | `led_toggle()` | main.c | 业务逻辑：每 500ms 翻转一次 |
| 驱动层 | `led_toggle()` | led_driver.c | 封装语义：LED 翻转 |
| HAL 层 | `hal_gpio_toggle()` | hal_gpio_stm32.c | 抽象接口：端口+引脚 |
| 厂商库 | `HAL_GPIO_TogglePin()` | stm32f1xx_hal_gpio.c | ST HAL 库实现 |
| 硬件层 | GPIOC->ODR 寄存器 | MCU 硬件 | 物理引脚电平翻转 |

---

### 2.5 CubeMX 配置步骤

#### 2.5.1 工程创建与芯片选型

1. 打开 STM32CubeMX，点击 **New Project**
2. 在 MCU 选择器中搜索 `STM32F103C8`，选择 `STM32F103C8Tx`
3. 点击 **Start Project**

#### 2.5.2 时钟配置

1. 在 **Pinout & Configuration** 视图中，进入 **System Core → RCC**
2. **High Speed Clock (HSE)**：选择 `Crystal/Ceramic Resonator`
3. 切换到 **Clock Configuration** 视图：
   - HSE 输入频率：8 MHz
   - PLL 倍频：×9 → HCLK = 72 MHz

**表 2-8** 时钟配置参数

| 配置项 | 值 | 说明 |
|--------|-----|------|
| HSE | 8 MHz | 外部晶振频率 |
| PLL 倍频 | ×9 | 8MHz × 9 = 72MHz |
| HCLK | 72 MHz | 系统主频 |
| APB1 Prescaler | /2 | APB1 总线 36MHz |
| APB1 Timer | ×2 | 定时器时钟 72MHz |

#### 2.5.3 GPIO 配置

**表 2-9** GPIO CubeMX 配置

| 引脚 | 配置 | 模式 | 用途 |
|------|------|------|------|
| PC13 | GPIO_Output | Push-Pull, High Speed | 板载 LED |
| PA13 | SYS_JTMS-SWDIO | Serial Wire | SWD 调试 |
| PA14 | SYS_JTCK-SWCLK | Serial Wire | SWD 调试 |

配置步骤：
1. 在 Pinout 视图中点击 **PC13** 引脚，选择 `GPIO_Output`
2. 进入 **System Core → GPIO**，设置 PC13 的用户标签为 `LED`
3. **SYS → Debug**：选择 `Serial Wire`

#### 2.5.4 工程生成

1. 进入 **Project Manager** 视图
2. 设置工程名称：`4250705021_wangyumeng_layered_arch`
3. 工具链选择：`STM32CubeIDE`
4. 点击 **Generate Code**

**表 2-10** 生成的工程目录结构

| 目录/文件 | 说明 |
|-----------|------|
| `Core/Inc/` | 头文件目录（main.h、hal_gpio.h、led_driver.h） |
| `Core/Src/` | 源文件目录（main.c、hal_gpio_stm32.c、led_driver.c） |
| `Core/Startup/` | 启动文件（startup_stm32f103c8tx.s） |
| `Drivers/` | HAL 库与 CMSIS 驱动 |
| `STM32F103C8TX_FLASH.ld` | 链接脚本 |

---

### 2.6 示例程序

#### 2.6.1 CubeMX 生成的初始化代码

CubeMX 自动生成的 GPIO 初始化函数位于 `Core/Src/gpio.c` 中：

```c
/**
 * @brief  GPIO 初始化函数（CubeMX 自动生成）
 * @note   配置 PC13 为推挽输出（板载 LED）
 */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 使能 GPIOC 时钟 */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 配置 PC13：推挽输出，高速 */
    GPIO_InitStruct.Pin   = LED_Pin;            /* GPIO_PIN_13 */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* 初始输出高电平（LED 熄灭） */
    HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET);
}
```

#### 2.6.2 完整工程文件清单

**表 2-11** 分层架构工程文件清单

| 文件 | 层次 | 说明 |
|------|------|------|
| `hal_gpio.h` | HAL 层 | 平台无关的 GPIO 接口声明 |
| `hal_gpio_stm32.c` | HAL 层 | STM32 平台的 GPIO 接口实现 |
| `led_driver.h` | 驱动层 | LED 驱动接口声明 |
| `led_driver.c` | 驱动层 | LED 驱动接口实现 |
| `main.c` | 应用层 | 业务逻辑（LED 闪烁） |
| `gpio.c/h` | 基础设施 | CubeMX 生成的 GPIO 初始化 |
| `stm32f1xx_it.c` | 基础设施 | 中断服务函数 |

#### 2.6.3 hal_gpio.h（完整源码）

```c
/**
 * @file    hal_gpio.h
 * @brief   硬件抽象层 — GPIO 接口定义
 * @note    平台无关，不包含任何芯片特定头文件
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

typedef enum {
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET   = 1
} GPIO_PinState;

void hal_gpio_write(uint8_t port, uint8_t pin, GPIO_PinState state);
void hal_gpio_toggle(uint8_t port, uint8_t pin);
GPIO_PinState hal_gpio_read(uint8_t port, uint8_t pin);

#endif /* HAL_GPIO_H */
```

#### 2.6.4 hal_gpio_stm32.c（完整源码）

```c
/**
 * @file    hal_gpio_stm32.c
 * @brief   HAL GPIO 接口的 STM32F1 平台实现
 */

#include "hal_gpio.h"
#include "stm32f1xx_hal.h"

static GPIO_TypeDef* port_map[] = { GPIOA, GPIOB, GPIOC };
#define PORT_MAP_SIZE  (sizeof(port_map) / sizeof(port_map[0]))

void hal_gpio_write(uint8_t port, uint8_t pin, GPIO_PinState state)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        HAL_GPIO_WritePin(port_map[port], (1U << pin), state);
    }
}

void hal_gpio_toggle(uint8_t port, uint8_t pin)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        HAL_GPIO_TogglePin(port_map[port], (1U << pin));
    }
}

GPIO_PinState hal_gpio_read(uint8_t port, uint8_t pin)
{
    if (port < PORT_MAP_SIZE && pin <= 15) {
        return (GPIO_PinState)HAL_GPIO_ReadPin(port_map[port], (1U << pin));
    }
    return GPIO_PIN_RESET;
}
```

#### 2.6.5 led_driver.h（完整源码）

```c
/**
 * @file    led_driver.h
 * @brief   LED 驱动层 — 基于 HAL GPIO 接口封装
 */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#define LED_PORT    2       /* GPIOC */
#define LED_PIN     13      /* PC13 */

void led_on(void);
void led_off(void);
void led_toggle(void);

#endif /* LED_DRIVER_H */
```

#### 2.6.6 led_driver.c（完整源码）

```c
/**
 * @file    led_driver.c
 * @brief   LED 驱动层实现
 */

#include "led_driver.h"
#include "hal_gpio.h"

void led_on(void)
{
    hal_gpio_write(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

void led_off(void)
{
    hal_gpio_write(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void led_toggle(void)
{
    hal_gpio_toggle(LED_PORT, LED_PIN);
}
```

#### 2.6.7 main.c（完整源码）

```c
/**
 * @file    main.c
 * @brief   STM32F103C8T6 分层架构 LED 闪烁示例
 * @note    CubeMX 生成框架 + 分层架构应用代码
 */

#include "main.h"
#include "gpio.h"
#include "led_driver.h"

/**
 * @brief  系统时钟配置（CubeMX 自动生成）
 *         HSE 8MHz → PLL ×9 → HCLK 72MHz
 */
void SystemClock_Config(void);

int main(void)
{
    /* ===== 系统初始化（CubeMX 生成）===== */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    /* ===== 应用逻辑（分层架构：仅调用驱动层接口）===== */
    while (1)
    {
        led_toggle();       /* LED 状态翻转 */
        HAL_Delay(500);     /* 延时 500ms，LED 闪烁周期 1s */
    }
}
```

---

### 2.7 PicSimLab 仿真验证

#### 2.7.1 仿真环境搭建

**表 2-12** 仿真环境配置

| 项目 | 配置 |
|------|------|
| 仿真器 | PicSimLab（QEMU 后端） |
| 板卡 | stm32_blue_pill |
| IDE | STM32CubeIDE 1.14+ |
| 固件格式 | .bin（Debug 目录下编译产物） |

#### 2.7.2 仿真操作步骤

**步骤 1：编译固件**

在 CubeIDE 中打开 `4250705021_wangyumeng_layered_arch` 工程，选择 **Debug** 配置，点击 **Build Project**（快捷键 Ctrl+B）。编译成功后在 `Debug/` 目录下生成 `4250705021_wangyumeng_layered_arch.bin`。

**步骤 2：启动 PicSimLab Docker 容器**

```bash
# 进入 PicSimLab 开发环境目录
cd code_examples/stm32_picsimlab_dev

# 启动 Docker 容器（首次运行会拉取镜像）
docker compose up -d

# 等待容器就绪
sleep 5
```

**步骤 3：加载固件并运行**

```bash
# 使用 run-firmware.sh 脚本一键加载固件
./run-firmware.sh ../exam2/4250705021_wangyumeng_layered_arch/Debug/4250705021_wangyumeng_layered_arch.bin

# 浏览器自动打开 PicSimLab 界面
# 或手动访问: http://127.0.0.1:6080/vnc.html?autoconnect=true
```

**步骤 4：观察仿真结果**

在 PicSimLab 界面中：
1. 确认板卡选择为 `stm32_blue_pill`
2. 观察 PC13 板载 LED 以约 1 秒周期闪烁（亮 500ms，灭 500ms）
3. 如需观察引脚波形，可添加 Oscilloscope 组件，探针连接 PC13

#### 2.7.3 接线与配置说明

**表 2-13** PicSimLab 仿真接线

| 组件 | 连接引脚 | 说明 |
|------|---------|------|
| 板载 LED | PC13 | Blue Pill 自带，低电平点亮 |
| 电源 | 3.3V / GND | 仿真器自动供电 |

#### 2.7.4 预期现象

**LED 闪烁效果：**

- PC13 板载 LED 以约 1 秒周期闪烁
- 亮 500ms，灭 500ms，亮度均匀
- 无抖动或异常闪烁

**波形分析：**

```bob
  PC13 引脚电平波形

  高电平 │    ┌───┐       ┌───┐       ┌───┐
         │    │   │       │   │       │   │
  低电平 │────┘   └───────┘   └───────┘   └───
         └────────────────────────────────────→ 时间
              500ms   500ms   500ms   500ms

         │←─ 周期 1s ─→│
```

**图 2-4** PC13 引脚电平波形：方波信号，周期 1 秒，占空比 50%。

#### 2.7.5 结果分析

**表 2-14** 仿真结果验证

| 验证项 | 理论值 | 仿真观测值 | 是否一致 |
|--------|--------|-----------|----------|
| LED 闪烁周期 | 1 秒 | 约 1 秒 | 一致 |
| LED 亮灭占比 | 50% / 50% | 均匀闪烁 | 一致 |
| 引脚电平 | PC13 高/低交替 | 示波器方波 | 一致 |
| 分层调用 | led_toggle → hal_gpio → HAL | 运行正常 | 一致 |

**分层架构验证结论：**

仿真结果表明，分层架构的调用链（应用层 → 驱动层 → HAL 层 → 硬件层）工作正常。LED 闪烁功能通过 `led_toggle()` → `hal_gpio_toggle()` → `HAL_GPIO_TogglePin()` 三级调用实现，各层职责清晰，接口调用正确。

---

### 2.8 拓展与思考

#### 2.8.1 多 LED 驱动扩展

分层架构的优势在多 LED 场景中更加明显。通过增加 LED 配置表，可以轻松管理多个 LED：

```c
/* led_driver.h — 多 LED 支持 */
typedef struct {
    uint8_t port;
    uint8_t pin;
    uint8_t active_low;  /* 1=低电平有效, 0=高电平有效 */
} LedConfig;

void led_on_by_id(uint8_t id);
void led_off_by_id(uint8_t id);
void led_toggle_by_id(uint8_t id);
```

**表 2-15** 多 LED 配置示例

| LED ID | 端口 | 引脚 | 有效电平 | 用途 |
|--------|------|------|---------|------|
| 0 | GPIOC | 13 | 低电平 | 板载 LED |
| 1 | GPIOA | 0 | 高电平 | 外接 LED 1 |
| 2 | GPIOA | 1 | 高电平 | 外接 LED 2 |

#### 2.8.2 增加 HAL 层测试能力

分层架构使得单元测试成为可能。可以编写 Mock HAL 实现，在无硬件环境下测试驱动逻辑：

```c
/* hal_gpio_mock.c — 用于单元测试的 Mock 实现 */
static GPIO_PinState mock_pins[3][16];  /* 模拟 GPIO 状态 */

void hal_gpio_write(uint8_t port, uint8_t pin, GPIO_PinState state)
{
    mock_pins[port][pin] = state;
}

GPIO_PinState hal_gpio_read(uint8_t port, uint8_t pin)
{
    return mock_pins[port][pin];
}

/* 测试用例 */
void test_led_on_sets_pin_low(void)
{
    led_on();
    assert(mock_pins[LED_PORT][LED_PIN] == GPIO_PIN_RESET);
}
```

#### 2.8.3 分层架构的适用场景

**表 2-16** 分层架构适用场景

| 场景 | 是否推荐分层 | 原因 |
|------|-------------|------|
| 多平台移植需求 | 强烈推荐 | HAL 层隔离硬件差异 |
| 团队协作开发 | 推荐 | 接口约定后各层独立开发 |
| 需要单元测试 | 推荐 | Mock HAL 层可脱离硬件测试 |
| 简单单片机项目 | 可选 | 如果仅使用一个平台，分层可能过度设计 |
| 资源极度受限 | 谨慎 | 函数调用层数增加会消耗少量栈空间 |

---

### 2.9 本章小结

本章以 STM32F103C8T6 为平台，系统讲解了嵌入式分层架构的设计思想与工程实践。

**表 2-17** 本章核心知识点汇总

| 知识点 | 内容 | 关键实现 |
|--------|------|---------|
| 分层架构 | 五层模型：应用/服务/驱动/HAL/硬件 | 单向依赖，职责分离 |
| HAL 接口 | 平台无关的 GPIO 抽象 | `hal_gpio.h` 纯接口声明 |
| HAL 实现 | STM32 平台的具体实现 | `hal_gpio_stm32.c` 端口映射 |
| LED 驱动 | 语义化封装 | `led_on/off/toggle` |
| 跨平台移植 | 仅替换 HAL 实现文件 | 应用层零修改 |
| 仿真验证 | PicSimLab 验证功能正确性 | LED 闪烁周期 1s |

**分层架构编程三步法：**

```bob
  ① 定义接口           ② 实现接口              ③ 使用接口
  hal_gpio.h           hal_gpio_stm32.c        led_driver.c → main.c
       |                      |                         |
       v                      v                         v
  平台无关声明           平台相关实现             业务逻辑调用
```

**图 2-5** 分层架构编程三步法：定义接口 → 实现接口 → 使用接口。

> **拓展阅读**：分层架构是嵌入式软件设计模式的基础。在后续学习中，读者可以将分层架构与状态机模式、观察者模式、生产者-消费者模式等结合使用，构建更加健壮和可维护的嵌入式系统。

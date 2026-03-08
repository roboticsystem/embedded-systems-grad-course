# 第七章 - STM32 定时器的应用

## 7.1 简介
定时器（Timer）是 STM32 外设中非常重要的部分，用于产生精确的时间基准、PWM 信号、输入捕获、编码器接口、事件触发以及与中断/DMA 协同工作。通过调整时钟分频（Prescaler）和自动重装值（ARR），可以灵活地控制计数周期与分辨率。

## 7.2 定时器基础概念
- 时钟来源：APB1 / APB2 总线时钟，经预分频后进入定时器。注意不同系列定时器的时钟倍频特性。
- 预分频器（Prescaler）：决定计数器时钟频率。计数器时钟 = 定时器时钟 / (Prescaler + 1)。
- 自动重装载寄存器（ARR）：决定计数周期，当计数器达到 ARR 时产生更新事件（Update Event）。
- 捕获/比较寄存器（CCR）：用于输入捕获或输出比较/PWM 的占空比设置。

## 7.3 计数模式
- 向上计数（Upcounting）
- 向下计数（Downcounting）
- 中心对齐模式（Center-aligned）：用于生成对称 PWM 或定时同步场景

## 7.4 输出比较与 PWM
输出比较 (Output Compare) 可用于生成 PWM 信号和定时输出。PWM 的占空比由 CCRx 与 ARR 的比值决定。

示例：基于 HAL 的简要 PWM 初始化与启动（伪代码）：

```c
// 假设使用 TIM2, 计数频率为 1MHz, 周期 1000 -> 1kHz PWM
TIM_HandleTypeDef htim2;
htim2.Instance = TIM2;
htim2.Init.Prescaler = 79;           // 如果 APB1 时钟 80MHz -> 计数时钟 1MHz
htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
htim2.Init.Period = 999;             // ARR
HAL_TIM_PWM_Init(&htim2);

TIM_OC_InitTypeDef sConfigOC = {0};
sConfigOC.OCMode = TIM_OCMODE_PWM1;
sConfigOC.Pulse = 500;               // CCRx -> 占空比 50%
sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);

HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
```

注意在实际工程中通常使用 STM32CubeMX 生成初始化代码并在此基础上修改 CCR 值实现占空比控制。

## 7.5 输入捕获（Input Capture）
输入捕获用于测量外部信号的频率、占空比或脉宽。基本思路是将外部事件捕获到 CCR 寄存器并读取捕获值，结合定时器时钟与预分频器计算真实时间。

示例流程（伪算法）：
1. 配置通道为输入捕获，设置触发边沿（上升沿/下降沿/双边沿）。
2. 在捕获中断或 DMA 回调中读取 CCR 值并计算时间间隔。
3. 根据计数器溢出与预分频计算最终频率/占空比。

```c
// 简单使用 HAL 获取两次上升沿计数差值
uint32_t last = 0, now = 0;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        now = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint32_t diff = (now >= last) ? (now - last) : (now + (htim->Init.Period + 1) - last);
        // diff 对应计数器刻度, 乘以刻度时间得脉宽
        last = now;
    }
}
```

## 7.6 编码器接口（Encoder Interface）
STM32 提供专用的编码器接口模式（Encoder Mode），可直接把两相正交编码器信号连接到定时器通道，从而高效读取旋转角度和速度。

配置要点：
- 将两个通道配置为 TI1/TI2 输入
- 选择合适的捕获极性和滤波器
- 使用 16/32 位定时器以满足计数范围需求

## 7.7 定时器中断与 DMA
- 更新中断（Update Interrupt）：ARR 溢出或手动更新时触发，常用于周期性任务。
- 捕获/比较中断（CCx）：在捕获或比较事件发生时触发，适用于精确定时处理。
- DMA：用于高频数据搬运（例如连续捕获、DAC 波形输出），减少 CPU 负担。

## 7.8 多定时器同步与高级功能
- 主从模式（Master/Slave）用于定时器间同步触发（例如同步 PWM 或采样同步）。
- 死区时间（Dead-time）与互补输出：用于半桥/全桥驱动，防止上下桥同时导通。
- 触发 ADC：定时器可作为 ADC 触发源实现精确采样同步。

## 7.9 调试与常见问题
- 时钟配置错误会导致定时器频率偏差，检查 RCC 与 APB 分频设置。
- 中断优先级与中断处理耗时会影响高频事件的可靠性，必要时采用 DMA。
- ARR/Prescaler 设置需避免计数器溢出并兼顾时间分辨率。

## 7.10 实验与练习
1. 使用 TIMx 生成固定频率的 PWM，修改占空比实现亮度调节（LED）。
2. 使用输入捕获测量外部方波的频率与占空比。
3. 配置编码器接口读取旋转角度并用串口输出。
4. 使用定时器触发 ADC 实现同步采样。

## 参考资料
- ST 官方参考手册（RM 系列）
- STM32 HAL/LL 库 API 文档
- 《STM32 嵌入式系统开发实战》 相关章节

---

（本章为概览性草稿，建议结合目标 MCU 数据手册与 HAL/LL 示例移植并补充具体芯片型号的寄存器细节与代码示例。）

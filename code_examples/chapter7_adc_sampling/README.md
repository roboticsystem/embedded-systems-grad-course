# chapter7_adc_sampling

《嵌入式系统》第 7 章 7.5 节——ADC 模拟采样示例工程。

## 硬件平台

- **MCU**：STM32F103C8T6（Blue Pill）
- **IDE**：STM32CubeIDE 1.x + CubeMX 6.x

## 功能说明

通过 `main.c` 顶部宏 **`ADC_USE_DMA`** 切换：

| 宏值 | 模式 | 说明 |
|:----:|------|------|
| `0` | 单通道轮询 | `ADC_Read()`，500 ms 打印 PA0 原始值与电压 |
| `1` | 三通道 DMA（默认） | `ADC_DMA_SampleBlock()`，200 ms 打印 IN0~IN2 毫伏值 |

## 引脚

| 引脚 | 功能 |
|------|------|
| PA0~PA2 | ADC1_IN0~IN2 |
| PA9 | USART1_TX（printf / VirtualTerm） |
| PA10 | USART1_RX |

串口：**115200，8N1**。

## 工程结构

```
chapter7_adc_sampling/
├── Core/Inc/、Core/Src/、Core/Startup/
├── Drivers/
├── adc_sampling.ioc
└── STM32F103C8TX_FLASH.ld
```

## 快速上手

1. CubeIDE 打开本目录，确认 `Core/Src/main.c` 中 `ADC_USE_DMA`（`1`=DMA，`0`=轮询）。
2. **Project → Build Project** 后烧录或用于 PicSimLab 仿真（教材 7.5.6 节）。

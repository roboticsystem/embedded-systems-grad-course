# 第 7.5 节截图清单

PNG 放入本目录，文件名与下表一致。

## 必截（5 张）

| 文件名 | 教材图号 | 在哪截 | 截什么 |
|--------|----------|--------|--------|
| `fig7-8_cubemx_adc_dma.png` | 图 7-7 | CubeIDE，`adc_sampling.ioc` 引脚图或 ADC1 页 | PA0~PA2 为 ADC1_INx，PA9 为 USART1_TX |
| `fig7-9_cubemx_dma_channel.png` | 图 7-8 | CubeIDE，`stm32f1xx_hal_msp.c` 中 `hdma_adc1` 初始化 | DMA1_Channel1、Normal、HalfWord、NVIC |
| `fig7-10_picsimlab_wiring.png` | 图 7-10 | PicSimLab 主窗 + Spare Parts | 3 电位器→PA0~2，VirtualTerm RXD→PA9 |
| `fig7-11_picsimlab_run.png` | 图 7-11 | VirtualTerm（**DMA 固件**，`ADC_USE_DMA=1`） | 首行 `===== 三通道 ADC DMA 扫描 =====` 及 `IN0/IN1/IN2=…mV` |
| `fig7-12_adc_poll_picsimlab.png` | 图 7-12 | VirtualTerm（**轮询固件**，`ADC_USE_DMA=0`） | `===== 单通道 ADC 轮询 =====` 及 `[n] ADC=… V=…` |

## PicSimLab 操作提示

- 加载固件：**File → Load Bin**（`.bin`，勿用 `.hex`）
- 启动仿真：工具栏 **Debug**
- 串口：**115200**，RXD 接 PA9

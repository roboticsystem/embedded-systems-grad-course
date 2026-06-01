# STM32 UART DMA Protocol Example

本工程对应课程 issue #118：`UART DMA 收发与协议帧编程`。目标平台为 `STM32F103C8T6 Blue Pill`，开发流程为 `STM32CubeMX + STM32CubeIDE + PicSimLab`。

## 功能

- USART1：`115200 8N1`
- RX：DMA1 Channel5，使用 `HAL_UARTEx_ReceiveToIdle_DMA()` 完成不定长接收
- TX：DMA1 Channel4，使用 `HAL_UART_Transmit_DMA()` 返回协议帧
- PC13：板载 LED，低电平点亮
- 协议帧：`AA 55 LEN CMD DATA CRC`

## CubeMX 配置

1. 打开 `STM32_UART_DMA_Protocol.ioc`。
2. 确认芯片为 `STM32F103C8Tx`。
3. 确认 USART1 异步模式，PA9 为 TX，PA10 为 RX。
4. 确认 DMA：
   - USART1_RX -> DMA1 Channel5，Peripheral to Memory
   - USART1_TX -> DMA1 Channel4，Memory to Peripheral
5. 确认 NVIC 启用 `USART1 global interrupt` 和 DMA 通道中断。
6. 使用 STM32CubeIDE 生成代码并编译。

## PicSimLab 运行

编译后在本地生成固件：

```bash
Debug/STM32_UART_DMA_Protocol.bin
```

启动仿真：

```bash
picsimlab --board=stm32_blue_pill --firmware=Debug/STM32_UART_DMA_Protocol.bin
```

在 UART Terminal 中设置 `115200 8N1` 和 HEX 输入模式，发送以下测试帧：

| 功能 | 请求帧 | 预期返回 |
|------|------|------|
| LED_ON | `AA 55 00 01 01` | `AA 55 02 80 01 00 83` |
| LED_OFF | `AA 55 00 02 02` | `AA 55 02 80 02 00 80` |
| ECHO "Hi" | `AA 55 02 03 48 69 20` | `AA 55 02 03 48 69 20` |
| Unknown CMD | `AA 55 00 09 09` | `AA 55 02 81 09 02 88` |

## 不提交的文件

以下文件由 CubeIDE 编译生成，不应提交到 Git：

- `Debug/`
- `Release/`
- `*.o`
- `*.d`
- `*.elf`
- `*.bin`
- `*.hex`
- `*.map`

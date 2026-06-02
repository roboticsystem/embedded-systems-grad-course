# PicSimLab 仿真

1. 板卡：Blue Pill (STM32F103C8)
2. 加载 CubeIDE 编译的 `.hex`（仅本地使用）
3. 添加 UART Terminal，115200，接 USART1 TX (PA9)
4. 复位后串口应输出 CONNECT / PUBLISH QoS0 / PUBLISH QoS1 三段十六进制

CONNECT 参考：

```text
10 11 00 04 4D 51 54 54 04 02 00 3C 00 05 73 74 6D 33 32
```

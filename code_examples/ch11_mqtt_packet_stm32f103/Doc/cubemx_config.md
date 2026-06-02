# CubeMX / CubeIDE 配置

| 项 | 值 |
|----|-----|
| MCU | STM32F103C8T6 |
| USART1 | 115200 8N1，PA9=TX，PA10=RX |

## 集成步骤

1. CubeMX 生成工程后，复制 `Core/Inc/mqtt_packet.h`、`Core/Src/mqtt_packet.c`、`Application/*` 到工程并加入编译。
2. 在 `main.c` 中：

```c
#include "mqtt_packet_demo.h"

/* USER CODE BEGIN 2 */
MQTT_PacketDemo_Run(&huart1);
/* USER CODE END 2 */
```

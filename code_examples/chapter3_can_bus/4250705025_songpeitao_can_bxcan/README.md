# STM32 bxCAN CAN 总线案例

本目录是第 3 章 3.10 节的配套轻量源码示例，用于演示 STM32F103 bxCAN 在 AGV 底盘电机控制场景中的初始化、发送、接收和滤波器配置。

该示例不是完整 CubeIDE 工程，刻意不提交 CubeMX 自动生成的 HAL/CMSIS 驱动库。使用时将 `.c` 文件复制到 CubeMX 工程的 `Core/Src/`，将 `.h` 文件复制到 `Core/Inc/`，再在 `main.c` 的用户代码区调用即可。

若需要直接导入 STM32CubeIDE 的完整工程，请使用相邻目录 `../4250705025_songpeitao_can_bxcan_project/`。

## 文件说明

| 文件 | 作用 | 对应验收点 |
|------|------|------------|
| `can_init_example.c` | 给出 `MX_CAN_Init()`，包含 `PCLK1`、Prescaler、BS1、BS2 和 1 Mbps 波特率计算 | CAN 初始化 C 代码含波特率配置 |
| `can_user.c` / `can_user.h` | 给出 `CAN_UserStart()`、`CAN_SendMotorCommand()`、FIFO0 接收回调 | CAN 发送/接收代码完整 |
| `can_user.c` / `can_user.h` | 给出接收全部、32 位掩码、16 位列表三种滤波器配置 | 滤波器高级配置代码 |
| `agv_chassis_demo.c` / `agv_chassis_demo.h` | 给出 AGV 底盘 4 电机 10 ms 周期控制、急停、反馈超时检查 | CAN 在嵌入式系统中的应用案例 |

## 硬件连接

- MCU：STM32F103C8T6 或其他带 bxCAN 的 STM32。
- CAN 收发器：TJA1050、MCP2551、SN65HVD230 等。
- 默认引脚：CAN_RX = PA11，CAN_TX = PA12。
- 若使用 PB8/PB9，需要在 CubeMX 中启用 CAN remap。
- 总线两端各接 120 Ω 终端电阻，所有节点共地。

## CubeMX 配置

1. `Connectivity -> CAN` 启用 CAN，模式选择 `Normal`。
2. `Clock Configuration` 中确认 `PCLK1 = 36 MHz`。
3. 位时序填写：
   - Prescaler = 4
   - BS1 = 6 TQ
   - BS2 = 2 TQ
   - SJW = 1 TQ
4. 计算结果：

```text
CAN bit rate = 36 MHz / 4 / (1 + 6 + 2) = 1 Mbps
Sample point = (1 + 6) / (1 + 6 + 2) = 77.8%
```

5. `NVIC Settings` 勾选 `CAN RX0 interrupt`。
6. 生成代码后，可参考 `can_init_example.c` 检查 CubeMX 生成的 `MX_CAN_Init()` 参数。如果工程已经有 CubeMX 生成的 `can.c`，不要同时编译 `can_init_example.c`，只复制其中的参数值。
7. 在 `main.c` 中加入：

```c
#include "can_user.h"
#include "agv_chassis_demo.h"

MX_CAN_Init();
CAN_UserStart();

while (1) {
    AGV_CAN_SetTargetRpm(1, 1200);
    AGV_CAN_SetTargetRpm(2, 1200);
    AGV_CAN_SetTargetRpm(3, 1200);
    AGV_CAN_SetTargetRpm(4, 1200);
    AGV_CAN_10msTask();
    HAL_Delay(10);
}
```

## 报文约定

| 方向 | CAN ID | DLC | 数据 |
|------|--------|-----|------|
| 主控到电机 | 0x201~0x204 | 4 | target_rpm[15:0]、enable、reserved |
| 电机到主控 | 0x211~0x214 | 6 | rpm[15:0]、current[15:0]、position[15:0] |
| 主控广播 | 0x100 | 1 | emergency_stop |

`CAN_UserStart()` 默认启用 `CAN_ConfigMotorRangeFilter()`，只接收 `0x210~0x21F` 范围内的标准帧，适合电机反馈报文。调试初期也可以改用 `CAN_FilterAcceptAll()`，确认硬件链路正常后再收紧滤波条件。

AGV 底盘应用层示例以 10 ms 为控制周期，通过 `AGV_CAN_10msTask()` 向四个电机发送目标转速。接收中断会把 `0x211~0x214` 的反馈报文解析到 `g_can_motor_feedback[]`，应用层可用 `AGV_CAN_IsMotorFeedbackFresh()` 判断节点是否超时。

## 验证建议

1. 先使用 `CAN_MODE_LOOPBACK` 验证发送、接收回调和解析逻辑。
2. 再切换到 `CAN_MODE_NORMAL`，连接两个 CAN 节点或 USB-CAN 分析仪。
3. 使用示波器检查 CAN_H/CAN_L 差分波形，确认终端电阻和地线连接。
4. 观察 `g_can_motor_feedback[]` 的 `last_tick` 是否持续刷新，判断反馈是否超时。

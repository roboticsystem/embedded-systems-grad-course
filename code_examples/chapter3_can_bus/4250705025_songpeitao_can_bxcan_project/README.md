# SongPeitao CAN AGV CubeIDE Project

本目录是第 3 章 3.10 节的完整 STM32CubeMX + STM32CubeIDE 工程示例，用于配合课程论文 #99 验证 bxCAN 初始化、HAL 收发、滤波器配置和 AGV 底盘 CAN 应用案例。

## 工程信息

| 项目 | 配置 |
|------|------|
| 学生 | 宋培桃 |
| 学号 | 4250705025 |
| MCU | STM32F103C8T6 |
| IDE | STM32CubeIDE 1.18.0 |
| 配置工具 | STM32CubeMX 6.14.0（DB.6.0.140） |
| 固件库 | STM32Cube FW_F1 V1.8.6 |
| CAN 引脚 | PA11 = CAN_RX, PA12 = CAN_TX |
| CAN 收发器 | TJA1050 / MCP2551 / SN65HVD230 |
| CAN 波特率 | 1 Mbps |
| 位时序 | PCLK1 = 36 MHz, Prescaler = 4, BS1 = 6 TQ, BS2 = 2 TQ |
| 上传版默认模式 | `CAN_SIMULATION_LOOPBACK = 0`，真实 CAN 总线 Normal 模式 |

```text
CAN bit rate = 36 MHz / 4 / (1 + 6 + 2) = 1 Mbps
Sample point = (1 + 6) / (1 + 6 + 2) = 77.8%
```

## 目录结构

```text
4250705025_songpeitao_can_bxcan_project/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── stm32f1xx_hal_conf.h
│   │   ├── stm32f1xx_it.h
│   │   ├── usart.h
│   │   ├── can_user.h
│   │   └── agv_chassis_demo.h
│   ├── Src/
│   │   ├── main.c
│   │   ├── can.c
│   │   ├── stm32f1xx_hal_msp.c
│   │   ├── stm32f1xx_it.c
│   │   ├── usart.c
│   │   ├── system_stm32f1xx.c
│   │   ├── syscalls.c
│   │   ├── sysmem.c
│   │   ├── can_user.c
│   │   └── agv_chassis_demo.c
│   └── Startup/
│       └── startup_stm32f103xb.s
├── Drivers/
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
├── SongPeitao_CAN_AGV.ioc
├── STM32F103C8TX_FLASH.ld
├── .project
├── .cproject
└── .mxproject
```

## 使用方法

1. 使用 STM32CubeIDE 1.18.0 打开本目录。
2. 打开 `SongPeitao_CAN_AGV.ioc`，检查 CAN、RCC、SYS 和 NVIC 配置。
3. 生成代码时保留 `USER CODE` 区域。
4. 编译工程并烧录到 STM32F103C8T6 开发板。
5. 连接 CAN 收发器，CAN_H/CAN_L 两端保留 120 Ω 终端电阻。

## PicSimLab 验证步骤

上传工程保持 Normal 模式，不包含编译产物。用于截图的本地测试副本建议放在 `D:\HuaweiMoveData\Users\27907\Desktop\350\picsimlab_can_local_test\4250705025_songpeitao_can_bxcan_project_softcan`，在该副本中把 `Core/Inc/main.h` 的 `CAN_SIMULATION_LOOPBACK` 和 `CAN_PICSIMLAB_SOFT_DEMO` 设置为 `1`。PicSimLab/QEMU 的 Blue Pill 板卡不完整仿真 STM32F103 bxCAN 寄存器，因此截图版使用软件回环日志证明应用层发送、反馈刷新和滤波器配置含义；上传版和实物板仍使用真实 HAL CAN 代码。

1. 在 STM32CubeIDE 1.18.0 中打开本地测试副本。
2. 编译工程，生成 `Debug/SongPeitao_CAN_AGV.elf` 和 `Debug/SongPeitao_CAN_AGV.bin`。这些文件只用于本地仿真，不提交到 Git。
3. 启动 PicSimLab，选择 Blue Pill / STM32F103C8T6 板卡并加载本地固件：

```bash
picsimlab --board "STM32F103C8T6" --program "Debug/SongPeitao_CAN_AGV.elf"
```

4. 若使用课程仓库的容器环境，可执行：

```bash
docker compose -f ../../stm32_picsimlab_dev/docker-compose.yml up --build
```

5. 打开 PicSimLab 的 Virtual Terminal，连接 USART1（PA9/PA10），波特率设为 9600。
6. 运行后观察日志：软件回环模式输出 `32MHz / 4 / (1 + 6 + 1) = 1Mbps`、`StdId 0x210-0x21F -> FIFO0`，随后按 100 ms 刷新 `0x211~0x214` 反馈数据，更新 `g_can_motor_feedback[]`。
7. 截图时至少保留两张：PicSimLab 板卡加载并运行的窗口；Virtual Terminal 中包含位时序、发送/接收 ID、滤波器范围和状态刷新的日志窗口。需要证明编译过程时，再补一张 STM32CubeIDE Build Console 截图。

## 截图材料

本工程随源码保留作业截图材料，位于 `screenshots/`：

- `01_cubemx_can_config.png`：CubeMX CAN 与引脚配置。
- `02_cubeide_ioc_config.png`：STM32CubeIDE 中的 CAN、USART1、PA11/PA12、PA9/PA10 配置。
- `03_cubeide_main_source.png`：主程序源码和仿真宏配置。
- `04_picsimlab_can_soft_loopback_result.png`：PicSimLab 运行结果，展示 VTerm 9600 串口输出和 CAN 软件回环状态刷新。

## 运行现象

- 上电后 `MX_CAN_Init()` 配置 bxCAN 为 1 Mbps 正常模式。
- `CAN_UserStart()` 启动 CAN，开启 FIFO0 接收中断，并启用 `0x210~0x21F` 标准数据帧滤波。
- 主循环每 10 ms 调用 `AGV_CAN_10msTask()`，向 `0x201~0x204` 发送四个电机目标转速。
- 收到 `0x211~0x214` 反馈后，中断回调解析转速、电流和位置到 `g_can_motor_feedback[]`。
- 若 200 ms 内没有收到某个电机反馈，应用层触发 `AGV_CAN_StopAllMotors()`，发送零转速和禁能命令。

## 提交范围说明

本工程提交完整源代码、启动文件、链接脚本、CubeMX 配置和必要 HAL/CMSIS 驱动文件；不提交任何编译产物。禁止提交的文件包括：

- `.o`
- `.d`
- `.elf`
- `.bin`
- `.hex`
- `Debug/`
- `Release/`

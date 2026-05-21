# SongPeitao CAN AGV CubeIDE Project

本目录是第 3 章 3.10 节的完整 STM32CubeMX + STM32CubeIDE 工程示例，用于配合课程论文 #99 验证 bxCAN 初始化、HAL 收发、滤波器配置和 AGV 底盘 CAN 应用案例。

## 工程信息

| 项目 | 配置 |
|------|------|
| 学生 | 宋培桃 |
| 学号 | 4250705025 |
| MCU | STM32F103C8T6 |
| IDE | STM32CubeIDE |
| 配置工具 | STM32CubeMX |
| CAN 引脚 | PA11 = CAN_RX, PA12 = CAN_TX |
| CAN 收发器 | TJA1050 / MCP2551 / SN65HVD230 |
| CAN 波特率 | 1 Mbps |
| 位时序 | PCLK1 = 36 MHz, Prescaler = 4, BS1 = 6 TQ, BS2 = 2 TQ |

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
│   │   ├── can_user.h
│   │   └── agv_chassis_demo.h
│   ├── Src/
│   │   ├── main.c
│   │   ├── can.c
│   │   ├── stm32f1xx_hal_msp.c
│   │   ├── stm32f1xx_it.c
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

1. 使用 STM32CubeIDE 打开本目录。
2. 打开 `SongPeitao_CAN_AGV.ioc`，检查 CAN、RCC、SYS 和 NVIC 配置。
3. 生成代码时保留 `USER CODE` 区域。
4. 编译工程并烧录到 STM32F103C8T6 开发板。
5. 连接 CAN 收发器，CAN_H/CAN_L 两端保留 120 Ω 终端电阻。

## PicSimLab 验证步骤

1. 在单板仿真阶段，可临时将 `Core/Src/can.c` 中的 `CAN_MODE_NORMAL` 改为 `CAN_MODE_LOOPBACK`，验证 bxCAN 内部收发路径。
2. 在 STM32CubeIDE 中构建工程，生成本地调试固件。`.elf/.bin/.hex` 仅用于本地运行，不提交到 Git。
3. 启动 PicSimLab 并加载固件：

```bash
picsimlab --board "STM32F103C8T6" --program "Debug/SongPeitao_CAN_AGV.elf"
```

4. 若使用课程仓库的容器环境，可执行：

```bash
docker compose -f ../../stm32_picsimlab_dev/docker-compose.yml up --build
```

5. 观察运行日志：初始化输出 1 Mbps 位时序，随后每 10 ms 发送 `0x201~0x204`，接收 `0x211~0x214` 反馈并刷新 `g_can_motor_feedback[]`。

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

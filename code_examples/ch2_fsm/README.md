# 第2章 状态机模式 - STM32 工程示例

## 项目概述

基于有限状态机（FSM）的智能门锁演示，目标板 **Blue Pill（STM32F103C8Tx）**，支持 PicSimLab 仿真。

- Switch-Case 实现（`fsm.h` 中切换宏）
- 表驱动实现（默认）

## 硬件配置

| 硬件模块 | 引脚 | 功能 |
|----------|------|------|
| 按键 | PA0、PA1、PA2 | 密码输入（上拉，低电平有效） |
| LED | PB0 | LOCKED |
| LED | PB1 | UNLOCKED |
| LED | PB2 | ALARM |
| 蜂鸣器 | PB3 | 报警声 |

## 工程结构

```
ch2_fsm/
├── STM32CubeMX/
│   ├── fsm_demo.ioc              # CubeMX 配置（F103C8）
│   ├── Core/                     # 应用源码 + HAL 配置
│   └── STM32CubeIDE/             # CubeIDE 工程（导入此目录）
├── Core/                         # 与 STM32CubeMX/Core 同步的源码副本
├── picsimlab/README.md           # PicSimLab 连线与启动说明
└── README.md
```

> `STM32CubeMX/Drivers/` 由 CubeMX **Generate Code** 生成，不纳入 Git（见 `.gitignore`）。

## 使用方法

### 1. 生成 HAL 并导入 CubeIDE

1. 用 **STM32CubeMX** 打开 `STM32CubeMX/fsm_demo.ioc`
2. **Project → Generate Code**（会生成 `STM32CubeMX/Drivers/`）
3. **STM32CubeIDE → Import** 目录：`STM32CubeMX/STM32CubeIDE/`
4. **Project → Build**，得到 `Debug/fsm_demo.hex`

### 2. PicSimLab 仿真

见 [picsimlab/README.md](picsimlab/README.md)：

- Board：**Blue Pill**
- 加载 `Debug/fsm_demo.hex`
- Parts：Push Buttons（PA0–PA2，Active=Low）+ LEDs（PB0–PB2，Active=High）

### 3. 操作说明

- 正确密码：依次按 **PA0 → PA1 → PA2**
- 连续 3 次错误 → ALARM（PB2 亮）
- 复位按钮回到 LOCKED

## 实现方式切换

在 `Core/Inc/fsm.h` 中：

```c
#define FSM_IMPLEMENTATION_SWITCH_CASE  0
#define FSM_IMPLEMENTATION_TABLE_DRIVEN 1
```

## 注意事项

- 不含编译产物（`.elf` / `.hex` / `Debug/`）
- 需 STM32CubeMX 6.x + CubeIDE + 本地 **STM32Cube FW_F1** 包
- PB3 作蜂鸣器输出，已在 `stm32f1xx_hal_msp.c` 中关闭 JTAG（`NOJTAG`）

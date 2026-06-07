# PicSimLab 仿真配置说明（Blue Pill / STM32F103C8）

## 启动命令

PICSimLab 官方命令格式：

```bash
picsimlab "Blue Pill" stm32f103c8t6 path/to/fsm_demo.hex path/to/fsm_demo.pcf
```

Windows 示例（按你的安装路径调整）：

```powershell
& "F:\PicsimLab\picsimlab.exe" "Blue Pill" stm32f103c8t6 `
  "G:\masterStudy\qianrushi\code_examples\ch2_fsm\STM32CubeMX\STM32CubeIDE\Debug\fsm_demo.hex"
```

或在 GUI 中：**Board → Blue Pill**，然后 **File → Load Hex/Binary** 加载固件。

## Parts 连线

| MCU 引脚 | 外设 | 功能 |
|---------|------|------|
| PA0 | 按键 1 + GND | 密码位 0 |
| PA1 | 按键 2 + GND | 密码位 1 |
| PA2 | 按键 3 + GND | 密码位 2 |
| PB0 | LED 1 + GND | 锁定指示 |
| PB1 | LED 2 + GND | 解锁指示 |
| PB2 | LED 3 + GND | 报警指示 |
| PB3 | 蜂鸣器 | 报警声音 |

在 Spare Parts 中添加 **Push Buttons** 和 **LEDs**，右键 **Properties** 或点击引脚名分配 MCU 引脚。

## 操作说明

1. 编译 CubeIDE 工程，生成 `Debug/fsm_demo.hex`（已启用 hex/bin 转换）
2. 启动 PICSimLab，选择 **Blue Pill**
3. 加载 hex，点击 **Run**
4. 初始 **LOCKED**：PB0 对应 LED 亮
5. 依次按 PA0 → PA1 → PA2，进入 **UNLOCKED**
6. 连续输错 3 次密码，进入 **ALARM**（PB2 亮，蜂鸣器响）

## 注意事项

1. 固件目标芯片为 **STM32F103C8Tx**，与 Blue Pill 一致
2. PB3 在 F103 上默认是 JTAG 引脚，代码中已通过 `__HAL_AFIO_REMAP_SWJ_NOJTAG()` 释放
3. `config.psl` 不是 PICSimLab 官方格式，可忽略；外设布局请用 **File → Save configuration** 存为 `.pcf`

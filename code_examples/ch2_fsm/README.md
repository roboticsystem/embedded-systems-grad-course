# 第2章 状态机模式 - STM32工程示例

## 项目概述

本工程实现了一个基于有限状态机（FSM）的智能门锁控制系统，包含两种实现方式：
- Switch-Case 实现
- 表驱动实现

## 硬件配置

| 硬件模块 | 引脚配置 | 功能说明 |
|----------|----------|----------|
| 按键输入 | PA0、PA1、PA2 | 密码输入按键 |
| LED输出 | PB0 | 锁定状态指示 |
| LED输出 | PB1 | 解锁状态指示 |
| LED输出 | PB2 | 报警状态指示 |
| 蜂鸣器 | PB3 | 报警声音输出 |

## 工程结构

```
ch2_fsm/
├── STM32CubeMX/
│   └── fsm_demo.ioc          # CubeMX配置文件
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── fsm.h
│   │   └── gpio.h
│   └── Src/
│       ├── main.c
│       ├── fsm.c
│       └── gpio.c
├── Drivers/
│   └── STM32F4xx_HAL_Driver/
│       └── Inc/
│           └── stm32f4xx_hal.h
└── README.md
```

## 状态机设计

### 状态定义
- `STATE_LOCKED` - 锁定状态（初始状态）
- `STATE_UNLOCKED` - 解锁状态
- `STATE_OPEN` - 门打开状态
- `STATE_ALARM` - 报警状态

### 事件定义
- `EVENT_PASSWORD_OK` - 密码正确
- `EVENT_PASSWORD_ERR` - 密码错误
- `EVENT_ERR_LIMIT` - 错误次数达到上限
- `EVENT_TIMEOUT` - 超时事件
- `EVENT_DOOR_PUSH` - 推门事件
- `EVENT_DOOR_CLOSE` - 门关闭事件
- `EVENT_ADMIN_RESET` - 管理员复位

## 使用方法

### 1. 使用CubeMX配置
1. 打开 `STM32CubeMX/fsm_demo.ioc`
2. 点击"Generate Code"生成工程
3. 使用CubeIDE打开生成的工程

### 2. 编译运行
1. 在CubeIDE中编译工程
2. 下载到STM32开发板
3. 观察LED状态指示

### 3. 操作说明
- 按下PA0-PA2按键输入密码
- 正确密码：PA0 -> PA1 -> PA2（依次按下）
- 错误密码：任意其他组合
- 连续3次错误触发报警
- 复位按钮可解除报警

## 实现方式切换

在 `main.c` 中通过宏定义选择实现方式：
```c
// 选择Switch-Case实现
#define FSM_IMPLEMENTATION_SWITCH_CASE 1

// 或选择表驱动实现
#define FSM_IMPLEMENTATION_TABLE_DRIVEN 1
```

## 注意事项

- 本工程不包含编译生成文件（.o、.d、.elf、.bin、.hex）
- 需使用STM32CubeMX 6.0+ 和 STM32CubeIDE 1.8+
- 建议使用STM32F407开发板进行测试
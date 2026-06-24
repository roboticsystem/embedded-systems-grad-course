# Ch8-LED：第 8.2 节 LED 指示灯实验

本目录包含《嵌入式系统》第 8 章 8.2 节的三个 LED 实验代码，适用于 STM32F103C8（Blue Pill）开发板。

## 实验内容

| 实验 | 功能 | 引脚 |
|------|------|------|
| 1 - 单 LED 控制 | 板载 PC13 LED 点亮/熄灭/翻转 | PC13 |
| 2 - 八路流水灯 | 8 个 LED 依次轮流点亮 | PA0~PA7 |
| 3 - PWM 呼吸灯 | LED 亮度从 0→100%→0 循环渐变 | PA0（TIM2_CH1） |

## 硬件连接

### 流水灯
将 8 个 LED（串联 220Ω 限流电阻）正极接 PA0~PA7，负极接 GND。

### 呼吸灯
将 LED（串联 220Ω 限流电阻）正极接 PA0，负极接 GND。PA0 复用为 TIM2_CH1。

## 构建与烧录

```bash
# 在 stm32_picsimlab_dev 容器中，或本地 arm-none-eabi-gcc 环境：
make          # 编译全部实验
make flash    # 使用 OpenOCD + ST-Link 烧录
make clean    # 清理构建产物
```

## 实验说明

编译后将生成 `ch8-led.hex` 文件。可通过以下方式运行：
- **真实硬件**：使用 ST-Link 烧录到 Blue Pill 开发板
- **PicSimLab 仿真**：运行 `run-firmware.sh ch8-led.hex` 加载到 PicSimLab 虚拟板
  （参考 `code_examples/stm32_picsimlab_dev/`）
*** End of File

# 第7章 红外传感器接口编程示例

《嵌入式系统》课程第7章配套示例代码 - 数字型与模拟型红外传感器接口编程。

## 功能说明

本示例演示了两种红外传感器的接口编程方法：

1. **数字输出型红外传感器（TCRT5000）**
   - 接口：GPIO 输入（PB0）
   - 功能：检测障碍物有无，控制 LED 状态

2. **模拟输出型红外传感器（GP2Y0A21）**
   - 接口：ADC 输入（PA0）
   - 功能：测量距离（10~80cm），输出电压转换为距离值

## 硬件连接

| 传感器引脚 | STM32 引脚 | 说明 |
|-----------|-----------|------|
| 数字红外 OUT | PB0 | GPIO 输入，上拉模式 |
| 模拟红外 OUT | PA0 | ADC1_IN0 |
| LED | PA5 | GPIO 输出，状态指示 |
| VCC | 3.3V | 电源正极 |
| GND | GND | 电源地 |

## CubeMX 配置

### ADC1 配置
- IN0 通道：启用
- 连续转换模式：Enable
- 数据对齐：Right alignment
- 采样时间：239.5 Cycles

### GPIO 配置
- PB0：GPIO_Input，Pull-up
- PA5：GPIO_Output

## 代码结构

```
Core/Src/main.c          - 主程序，包含红外传感器驱动函数
  ├─ IR_Digital_IsObstacle()      // 数字红外障碍物检测
  ├─ IR_Analog_ReadRaw()          // 读取 ADC 原始值
  ├─ IR_Analog_ReadVoltage()      // 转换为电压值
  └─ IR_Analog_GetDistance()      // 计算距离（cm）
```

## 运行效果

1. **数字红外传感器**
   - 检测到障碍物时 LED 点亮
   - 无障碍物时 LED 熄灭

2. **模拟红外传感器**
   - 返回距离值（10~80cm）
   - 超出范围返回 -1

## PicSimlab 仿真

1. 启动 PicSimlab，选择 STM32F103C8T6
2. 添加虚拟 Ir 组件，连接到 PB0
3. 加载本工程编译的 .hex 文件
4. 运行并观察 LED 状态变化

## 编译说明

使用 STM32CubeIDE 或 Keil MDK 打开项目，直接编译即可。

## 作者

课程论文示例代码 - 第7章 红外传感器接口编程

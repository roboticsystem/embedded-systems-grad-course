# 嵌入式系统期末大报告：基于STM32的独立看门狗(IWDG)与Flash读写操作实现

## 一、 实验目的与背景
1. 掌握STM32微控制器的独立看门狗（IWDG）的工作原理及配置方法，实现系统异常时的自动复位，提升系统的鲁棒性。
2. 掌握STM32内部Flash的擦除、写入与读取操作，实现关键数据的非易失性存储。
3. 熟练使用STM32CubeIDE进行工程配置、代码编写与编译。
4. 掌握使用PICSimLab进行嵌入式系统的虚拟仿真与调试。

## 二、 实验环境
- **硬件平台**：STM32F103C8T6 (Blue Pill) - 仿真环境
- **软件环境**：
  - 操作系统：Windows
  - 开发工具：STM32CubeIDE / VS Code
  - 仿真工具：PICSimLab
  - 固件库：STM32 HAL库

## 三、 核心功能设计与实现

### 3.1 独立看门狗 (IWDG) 配置与实现
独立看门狗由专用的低速时钟（LSI）驱动，即使主时钟发生故障它也仍然有效。
在 `main.c` 中，我们通过直接操作寄存器的方式初始化了IWDG，并设置了重装载值。

**关键代码：**
```c
// IWDG 初始化
static void IWDG_Init(void)
{
  IWDG->KR = 0x5555U; // 解除写保护
  IWDG->PR = 0x04U;   // 预分频器设置，64分频
  IWDG->RLR = IWDG_RELOAD_VALUE; // 设置重装载值
  while ((IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) != 0U)
  {
    // 等待状态寄存器更新完成
  }
  IWDG->KR = 0xAAAAU; // 喂狗，刷新计数器
  IWDG->KR = 0xCCCCU; // 启动看门狗
}

// 喂狗操作
static void IWDG_Feed(void)
{
  IWDG->KR = 0xAAAAU; // 重新加载计数值，防止复位
}
```

### 3.2 Flash 读写与擦除操作
STM32的内部Flash除了存储程序代码外，还可以用于存储用户数据。在写入数据前，必须先解锁Flash并擦除目标页。

**关键代码：**
```c
// Flash 写入操作
static AppStatus_t Flash_WriteWords(uint32_t address, const uint32_t *data, uint32_t word_count)
{
  FLASH_EraseInitTypeDef erase = {0}; 
  uint32_t page_error = 0U, i;
  
  if (HAL_FLASH_Unlock() != HAL_OK) return APP_FLASH_WRITE_ERROR; // 解锁Flash
  
  erase.TypeErase = FLASH_TYPEERASE_PAGES; 
  erase.PageAddress = address; 
  erase.NbPages = 1;
  
  // 擦除Flash页
  if (HAL_FLASHEx_Erase(&erase, &page_error) != HAL_OK) { 
      HAL_FLASH_Lock(); 
      return APP_FLASH_ERASE_ERROR; 
  }
  
  // 逐字写入数据
  for (i = 0; i < word_count; i++)
  {
    IWDG_Feed(); // 写入过程耗时较长，需及时喂狗防止复位
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + (i * 4U), data[i]) != HAL_OK)
    { 
        HAL_FLASH_Lock(); 
        return APP_FLASH_WRITE_ERROR; 
    }
  }
  HAL_FLASH_Lock(); // 重新锁定Flash
  return APP_OK;
}

// Flash 读取操作
static void Flash_ReadWords(uint32_t address, uint32_t *data, uint32_t word_count)
{
  uint32_t i; 
  for (i = 0; i < word_count; i++) 
      data[i] = *(__IO uint32_t *)(address + (i * 4U)); // 直接通过指针读取Flash地址数据
}
```

### 3.3 主程序逻辑与错误处理
在 `main()` 函数中，系统初始化后首先进行Flash的读写测试，如果测试失败则进入 `Error_Handler()`，LED快速闪烁指示错误。如果成功，则进入主循环，定期喂狗并翻转LED指示系统正常运行。

**关键代码：**
```c
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  IWDG_Init(); // 初始化看门狗
  
  // 执行Flash读写测试
  if (App_RunFlashDemo() != APP_OK)
  {
    Error_Handler(); // 错误处理
  }
  
  // 测试成功，LED常亮100ms后进入主循环
  HAL_GPIO_WritePin(APP_LED_GPIO_Port, APP_LED_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(APP_LED_GPIO_Port, APP_LED_Pin, GPIO_PIN_SET);

  while (1)
  {
    IWDG_Feed(); // 喂狗
    HAL_GPIO_TogglePin(APP_LED_GPIO_Port, APP_LED_Pin); // 翻转LED，指示系统运行
    HAL_Delay(100);
  }
}
```

## 四、 实验步骤与仿真验证

### 4.1 代码编译
在开发环境中完成代码编写后，使用交叉编译工具链对工程进行编译，生成 `.bin` 和 `.elf` 固件文件。编译过程无错误，成功生成 `WatchDoorDog.bin`。

### 4.2 PICSimLab 仿真配置与运行
1. 打开 PICSimLab 仿真软件。
2. 在 `Board` 菜单中选择 `Blue Pill` (STM32F103C8T6) 开发板。
3. 在 `File` -> `Load Hex` 中加载编译生成的 `WatchDoorDog.bin` 文件。
4. 观察开发板上的 LED 状态。LED 正常闪烁，说明 Flash 读写测试通过，且 IWDG 喂狗正常，系统未发生异常复位。

**仿真运行截图：**
![PICSimLab 仿真运行截图](./screenshots/picsimlab_running.png)

## 五、 实验总结
本次实验成功在 STM32F103C8T6 平台上实现了独立看门狗（IWDG）的配置与内部 Flash 的读写操作。通过直接操作寄存器实现了 IWDG 的高效配置，并结合 HAL 库完成了 Flash 的页擦除与字写入。在 Flash 写入的耗时操作中，加入了喂狗机制，有效避免了因操作超时导致的系统误复位。通过 PICSimLab 仿真验证，系统逻辑正确，运行稳定，达到了预期的实验目标。本实验加深了对嵌入式系统底层硬件操作及系统可靠性设计的理解。

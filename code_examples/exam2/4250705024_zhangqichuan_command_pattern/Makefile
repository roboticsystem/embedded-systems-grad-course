# ==========================================================================
# Makefile - STM32F103C8T6 命令模式固件交叉编译
# 仅用于本地 / Docker 验证工程可编译，产物不入库（见 .gitignore）
# ==========================================================================
TARGET = firmware

# ---------- 工具链 ----------
CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc -x assembler-with-cpp
CP      = arm-none-eabi-objcopy
SZ      = arm-none-eabi-size

# ---------- MCU 参数：Cortex-M3，硬件不带 FPU ----------
MCU = -mcpu=cortex-m3 -mthumb

# ---------- 宏定义 ----------
DEFS = -DSTM32F103xB -DUSE_HAL_DRIVER

# ---------- 目录 ----------
BUILD_DIR = build
DRV_DIR   = Drivers/STM32F1xx_HAL_Driver
CMSIS_DIR = Drivers/CMSIS

# ---------- 应用层源码（Core/Src 全部 .c）----------
C_SOURCES = \
  Core/Src/main.c \
  Core/Src/gpio.c \
  Core/Src/usart.c \
  Core/Src/command.c \
  Core/Src/stm32f1xx_it.c \
  Core/Src/stm32f1xx_hal_msp.c \
  Core/Src/system_stm32f1xx.c \
  Core/Src/syscalls.c \
  Core/Src/sysmem.c

# ---------- 所用 HAL 驱动模块 ----------
C_SOURCES += \
  $(DRV_DIR)/Src/stm32f1xx_hal.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_cortex.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_rcc.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_rcc_ex.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_gpio.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_uart.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_pwr.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_flash.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_flash_ex.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_exti.c \
  $(DRV_DIR)/Src/stm32f1xx_hal_dma.c

# ---------- 启动文件 ----------
ASM_SOURCES = Core/Startup/startup_stm32f103c8tx.s

# ---------- 头文件搜索路径 ----------
C_INCLUDES = \
  -ICore/Inc \
  -I$(DRV_DIR)/Inc \
  -I$(CMSIS_DIR)/Device/ST/STM32F1xx/Include \
  -I$(CMSIS_DIR)/Include

# ---------- 编译/汇编选项 ----------
CFLAGS = $(MCU) $(DEFS) $(C_INCLUDES) -Wall -O0 -g3 -ffunction-sections -fdata-sections -std=gnu11
ASFLAGS = $(MCU) -g3

# ---------- 链接选项 ----------
LDSCRIPT = STM32F103C8TX_FLASH.ld
LIBS    = -lc -lm -lnosys
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBS) -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O binary $< $@
	cp $@ firmware.bin

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) firmware.bin

.PHONY: all clean

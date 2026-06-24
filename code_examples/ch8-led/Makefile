# Ch8-LED Makefile — 适用于 arm-none-eabi-gcc + STM32F103C8

TARGET   = ch8-led
CPU      = -mcpu=cortex-m3 -mthumb
CFLAGS   = $(CPU) -Wall -Wextra -O2 -ffreestanding -nostdlib
LDFLAGS  = $(CPU) -T stm32f103.ld -Wl,--gc-sections -Wl,-Map=$(TARGET).map
ASFLAGS  = $(CPU)

CMSIS    = ./cmsis
SRC_DIR  = ./src
BUILD_DIR = ./build

C_SRCS   = $(SRC_DIR)/main.c $(SRC_DIR)/led.c
ASM_SRCS = $(SRC_DIR)/startup_stm32f103.s
OBJS     = $(addprefix $(BUILD_DIR)/, $(notdir $(C_SRCS:.c=.o) $(ASM_SRCS:.s=.o)))

all: $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	arm-none-eabi-gcc $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	arm-none-eabi-as $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) stm32f103.ld
	arm-none-eabi-gcc $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	arm-none-eabi-objcopy -O ihex $< $@

# OpenOCD 烧录 (ST-Link)
flash: $(BUILD_DIR)/$(TARGET).hex
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "program $(BUILD_DIR)/$(TARGET).hex verify reset exit"

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all flash clean
*** End of File

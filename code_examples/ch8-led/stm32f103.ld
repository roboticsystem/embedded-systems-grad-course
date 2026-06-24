/**
 * stm32f103.ld — STM32F103C8T6 链接脚本
 *
 * 存储器映射：
 *   Flash: 0x08000000, 64KB
 *   SRAM:  0x20000000, 20KB
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

ENTRY(Reset_Handler)

SECTIONS
{
    .vectors : {
        . = ALIGN(4);
        KEEP(*(.vectors))
        . = ALIGN(4);
    } > FLASH

    .text : {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    _sidata = .;

    .data : AT(_sidata) {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM

    .bss : {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    /* 栈顶：SRAM 末尾 */
    . = ALIGN(8);
    _estack = ORIGIN(SRAM) + LENGTH(SRAM);
}
*** End of File

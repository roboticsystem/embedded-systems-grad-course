/**
 * startup_stm32f103.s — STM32F103C8 启动文件
 *
 * 功能：设置栈指针、初始化 .data / .bss 段、跳转 main。
 * 适用于 arm-none-eabi-gcc + stm32f103.ld。
 */

.syntax unified
.cpu    cortex-m3
.thumb

/* 栈顶地址（链接脚本中定义）*/
.word   _estack
.word   Reset_Handler
.word   NMI_Handler
.word   HardFault_Handler
.word   MemManage_Handler
.word   BusFault_Handler
.word   UsageFault_Handler
.word   0, 0, 0, 0          /* Reserved */
.word   SVC_Handler
.word   DebugMon_Handler
.word   0                   /* Reserved */
.word   PendSV_Handler
.word   SysTick_Handler

/* 中断向量表（仅列出基础异常，外部中断未用）*/
.globl  g_pfnVectors
g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .rept  10
    .word 0
    .endr
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler
    .rept  48                /* IRQ 0~47 */
    .word Default_Handler
    .endr

/* 复位入口：初始化段后跳转 main */
.thumb_func
Reset_Handler:
    ldr   r0, =_sdata
    ldr   r1, =_edata
    ldr   r2, =_sidata
    movs  r3, #0
    b     .L_copy_data
.L_loop_data:
    ldr   r4, [r2, r3]
    str   r4, [r0, r3]
    adds  r3, #4
.L_copy_data:
    adds  r4, r0, r3
    cmp   r4, r1
    bcc   .L_loop_data

    ldr   r0, =_sbss
    ldr   r1, =_ebss
    movs  r3, #0
    b     .L_zero_bss
.L_loop_bss:
    str   r3, [r0]
    adds  r0, #4
.L_zero_bss:
    cmp   r0, r1
    bcc   .L_loop_bss

    bl    main
    b     .

/* 默认中断处理：死循环 */
.thumb_func
Default_Handler:
NMI_Handler:
HardFault_Handler:
MemManage_Handler:
BusFault_Handler:
UsageFault_Handler:
SVC_Handler:
DebugMon_Handler:
PendSV_Handler:
SysTick_Handler:
    b     .

.end
*** End of File

/**
 * @file    command.c
 * @brief   命令模式核心实现：环形缓冲 + 函数指针命令表 + 行解析分发
 * @note    Invoker = cmd_process/cmd_dispatch；Receiver = cmd_led/cmd_help/cmd_ver；
 *          Command 结构体即命令模式中的"命令对象"，cmd_table 为查找表。
 */
#include "command.h"
#include "main.h"
#include "usart.h"
#include <string.h>

/* ---------- 接收环形缓冲（中断写、主循环读，无锁单生产者单消费者）---------- */
static volatile uint8_t rb_buf[CMD_RB_SIZE];
static volatile uint16_t rb_head = 0, rb_tail = 0;

void cmd_rx_feed(uint8_t byte) {                 /* 中断上下文：仅入队 */
    uint16_t next = (rb_head + 1) & (CMD_RB_SIZE - 1);
    if (next != rb_tail) {                       /* 未满才写，满则丢弃 */
        rb_buf[rb_head] = byte;
        rb_head = next;
    }
}

static int rb_get(uint8_t *out) {                /* 主循环：出队一字节 */
    if (rb_head == rb_tail) return 0;
    *out = rb_buf[rb_tail];
    rb_tail = (rb_tail + 1) & (CMD_RB_SIZE - 1);
    return 1;
}

static void uart_puts(const char *s) {           /* 阻塞回显 */
    HAL_UART_Transmit(&huart1, (uint8_t *)s, strlen(s), 100);
}

/* ---------- 命令处理函数（Receiver 角色）---------- */
static void cmd_led(int argc, char **argv) {     /* led on|off|toggle */
    if (argc < 2) { uart_puts("usage: led on|off|toggle\r\n"); return; }
    /* PC13 低电平点亮：on→拉低，off→拉高，toggle→翻转 */
    if      (!strcmp(argv[1], "on"))     HAL_GPIO_WritePin(LED_ONBOARD_GPIO_Port, LED_ONBOARD_Pin, GPIO_PIN_RESET);
    else if (!strcmp(argv[1], "off"))    HAL_GPIO_WritePin(LED_ONBOARD_GPIO_Port, LED_ONBOARD_Pin, GPIO_PIN_SET);
    else if (!strcmp(argv[1], "toggle")) HAL_GPIO_TogglePin(LED_ONBOARD_GPIO_Port, LED_ONBOARD_Pin);
    else { uart_puts("unknown arg\r\n"); return; }
    uart_puts("ok\r\n");
}
static void cmd_help(int argc, char **argv);     /* 前置声明，遍历命令表 */
static void cmd_ver(int argc, char **argv) { (void)argc; (void)argv; uart_puts("cmdshell v1.0 STM32F103\r\n"); }

static const Command cmd_table[] = {             /* 函数指针表：扩展命令只改表 */
    { "led",  cmd_led,  "led on|off|toggle" },
    { "help", cmd_help, "show commands" },
    { "ver",  cmd_ver,  "firmware version" },
};
#define CMD_COUNT (sizeof(cmd_table) / sizeof(cmd_table[0]))

static void cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    for (unsigned i = 0; i < CMD_COUNT; i++) {
        uart_puts(cmd_table[i].name); uart_puts("\t- ");
        uart_puts(cmd_table[i].help); uart_puts("\r\n");
    }
}

static void cmd_dispatch(char *line) {           /* 解析参数并查表分发 */
    char *argv[CMD_ARGS_MAX]; int argc = 0;
    char *p = strtok(line, " ");
    while (p && argc < CMD_ARGS_MAX) { argv[argc++] = p; p = strtok(NULL, " "); }
    if (argc == 0) return;
    for (unsigned i = 0; i < CMD_COUNT; i++)     /* 查找逻辑：遍历命令表匹配名字 */
        if (!strcmp(argv[0], cmd_table[i].name)) { cmd_table[i].handler(argc, argv); return; }
    uart_puts("unknown command, type 'help'\r\n");
}

/* ---------- 行组装：从环形缓冲取字节，遇 \r/\n 结束 ---------- */
void cmd_process(void) {
    static char line[CMD_LINE_MAX];
    static uint16_t idx = 0;
    static uint8_t  overflow = 0;                /* 溢出标志 */
    uint8_t c;
    while (rb_get(&c)) {
        if (c == '\r' || c == '\n') {
            if (overflow) { uart_puts("line too long\r\n"); }
            else if (idx > 0) { line[idx] = '\0'; cmd_dispatch(line); }
            idx = 0; overflow = 0;               /* 复位行状态 */
        } else if (idx < CMD_LINE_MAX - 1) {
            line[idx++] = (char)c;               /* 正常收字符 */
        } else {
            overflow = 1;                        /* 超长：置位并丢弃，杜绝越界写 */
        }
    }
}

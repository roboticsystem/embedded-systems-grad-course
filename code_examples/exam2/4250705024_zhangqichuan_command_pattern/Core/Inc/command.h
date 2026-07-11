#ifndef COMMAND_H
#define COMMAND_H
#include <stdint.h>

#define CMD_LINE_MAX   32     /* 单行命令最大长度，超长丢弃（溢出防护）*/
#define CMD_ARGS_MAX   4      /* 最多参数个数 */
#define CMD_RB_SIZE    64     /* 接收环形缓冲大小，须为 2 的幂 */

typedef void (*CmdHandler)(int argc, char **argv);   /* 命令处理函数指针 */

typedef struct {
    const char *name;      /* 命令名 */
    CmdHandler  handler;   /* 处理函数 */
    const char *help;      /* 帮助文本 */
} Command;

void cmd_rx_feed(uint8_t byte);   /* 由 UART 中断调用，逐字节入队 */
void cmd_process(void);           /* 由主循环调用，解析并分发一行命令 */

#endif /* COMMAND_H */

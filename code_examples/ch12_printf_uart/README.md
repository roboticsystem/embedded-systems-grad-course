# ch12_printf_uart — Issue #125 printf 串口重定向

课程教材第 12.5 节对应的可编译工程。本地开发工程名为 **`printf`**，提交到本仓库时请复制源码（不要复制编译产物）。

## 本地工程位置（作者环境）

```
C:\Users\yjq\STM32CubeIDE\workspace_1.13.2\printf\
├── printf.ioc
├── Core/Src/main.c          ← fputc、_write、printf 主循环
├── Core/Inc/main.h
└── Debug/printf.bin         ← 仅本地/PicSimLab 使用，勿提交 Git
```

## 复制到本目录（提交 PR 前）

在 PowerShell 中示例（请按实际路径调整）：

```powershell
$src = "$env:USERPROFILE\STM32CubeIDE\workspace_1.13.2\printf"
$dst = "code_examples\ch12_printf_uart"
# 复制 Core、Drivers、printf.ioc、.project、.cproject 等，勿复制 Debug/Release
```

至少应包含：`printf.ioc`、`Core/`、IDE 工程文件；**禁止**提交 `Debug/`、`*.elf`、`*.bin`、`*.hex`、`*.o`。

## 编译与仿真

1. CubeIDE：**Build Project** → `Debug/printf.bin`
2. PicSimLab：板卡 `stm32_blue_pill`，加载 `printf.bin`，UART Terminal **115200**
3. 预期输出（每秒一行）：`Hello UART1! printf redirect successful`

详见 `docs/chapter12.md` 第 12.5 节。

#ifndef SHELL_H
#define SHELL_H

#define ANSI_CURSOR_FORWARD(n) printf("\x1b[%dC", n)
#define ANSI_CURSOR_BACKWARD(n) printf("\x1b[%dD", n)
#define ANSI_CLEAR_LINE() printf("\x1b[2K\r")

#define HISTFILE "/.mos_history" // 历史文件路径
#define HISTFILESIZE 20         // 历史指令最大数量

void add_to_history(const char *cmd);

struct Builtin {
	const char *name; // 指令名
	int (*handler)(int argc, char **argv); // 处理函数指针
};

#endif
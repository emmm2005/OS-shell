#include <args.h>
#include <lib.h>
#include <shell.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

char history_commands[HISTFILESIZE][1024]; // 假设每条指令最大1024字节
int history_count = 0;					   // 当前历史指令数量
int history_index = 0;					   // 当前用户在历史记录中的位置 (用于 Up/Down)

int builtin_history(int argc, char **argv);
int builtin_cd(int argc, char **argv);
int builtin_pwd(int argc, char **argv);
int builtin_exit(int argc, char **argv);
int builtin_declare(int argc, char **argv);
int builtin_unset(int argc, char **argv);
// extern int resolve_path(const char *old_path, char *resolved_path);

int envid;

struct Builtin builtins[] = {
	{"history", builtin_history},
	{"pwd", builtin_pwd},
	{"cd", builtin_cd},
	{"exit", builtin_exit},
	{"declare", builtin_declare},
	{"unset", builtin_unset},
	{NULL, NULL} // 数组结束标记
};

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2)
{
	*p1 = 0;
	*p2 = 0;
	if (s == 0)
	{
		return 0;
	}

	while (strchr(WHITESPACE, *s))
	{
		*s++ = 0;
	}
	if (*s == 0)
	{
		return 0;
	}

	if (strchr(SYMBOLS, *s))
	{
		int t = *s;
		*p1 = s;
		*s++ = 0;
		if (t == '>' && *s == '>')
		{
			t = 's'; // replace >>
			*s++;
		}
		else if (t == '&' && *s == '&')
		{
			t = 'y'; // replace &&
			*s++;
		}
		else if (t == '|' && *s == '|')
		{
			t = 'h'; // replace ||
			*s++;
		}

		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s))
	{
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1)
{
	static int c, nc;
	static char *np1, *np2;

	if (s)
	{
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe)
{
	int argc = 0;
	while (1)
	{
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		switch (c)
		{
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS)
			{
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w')
			{
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			// If the 'open' function encounters an error,
			// utilize 'debugf' to print relevant messages,
			// and subsequently terminate the process using 'exit'.
			/* Exercise 6.5: Your code here. (1/3) */
			char old_path[1024];
			char path[1024];
			syscall_get_cwd(0, path);
			// printf("%s\n", path);
			strcpy(old_path, t);
			// printf("%s\n", old_path);
			resolve_path(old_path, path);
			// printf("%s\n", path);
			fd = open(path, O_RDONLY);
			if (fd < 0)
			{
				debugf("failed to open '%s'\n", t);
				exit();
			}
			dup(fd, 0);
			close(fd);

			// user_panic("< redirection not implemented");

			break;
		case '>':
			if (gettoken(0, &t) != 'w')
			{
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, create it if not exist and trunc it if exist, dup
			// it onto fd 1, and then close the original fd.
			// If the 'open' function encounters an error,
			// utilize 'debugf' to print relevant messages,
			// and subsequently terminate the process using 'exit'.
			/* Exercise 6.5: Your code here. (2/3) */
			char old_path1[1024];
			char path1[1024];
			syscall_get_cwd(0, path1);
			// printf("%s\n", path);
			strcpy(old_path1, t);
			// printf("%s\n", old_path);
			resolve_path(old_path1, path1);
			// printf("%s\n", path);
			fd = open(path1, O_WRONLY | O_CREAT | O_TRUNC);
			if (fd < 0)
			{
				debugf("failed to open '%s'\n", t);
				exit();
			}
			dup(fd, 1);
			close(fd);

			// user_panic("> redirection not implemented");

			break;
		case 's':
			if (gettoken(0, &t) != 'w')
			{
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, create it if not exist and trunc it if exist, dup
			// it onto fd 1, and then close the original fd.
			// If the 'open' function encounters an error,
			// utilize 'debugf' to print relevant messages,
			// and subsequently terminate the process using 'exit'.
			/* Exercise 6.5: Your code here. (2/3) */
			char old_path2[1024];
			char path2[1024];
			syscall_get_cwd(0, path2);
			// printf("%s\n", path);
			strcpy(old_path2, t);
			// printf("%s\n", old_path);
			resolve_path(old_path2, path2);
			// printf("%s\n", path);
			fd = open(path2, O_WRONLY | O_CREAT | O_APPEND);
			if (fd < 0)
			{
				debugf("failed to open '%s'\n", t);
				exit();
			}
			dup(fd, 1);
			close(fd);
			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			r = pipe(p);
			if (r != 0)
			{
				debugf("pipe: %d\n", r);
				exit();
			}
			r = fork();
			if (r < 0)
			{
				debugf("fork: %d\n", r);
				exit();
			}
			*rightpipe = r;
			if (r == 0)
			{
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			}
			else
			{
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}

			// user_panic("| not implemented");

			break;
		case ';':
			int child = fork();
			if (child < 0)
			{
				exit();
			}
			else if (child == 0)
			{
				return argc;
			}
			else
			{
				wait(child);
				return parsecmd(argv, rightpipe);
			}
			break;
		case 'y':
			int childand = fork();
			if (childand < 0)
			{
				exit();
			}
			else if (childand == 0)
			{
				return argc;
			}
			else
			{
				wait(childand);
				if (strcmp(argv[0], "/mkdir"))
				{
					return parsecmd(argv, rightpipe);
				}
				else
				{
					return 0;
				}
			}
			break;
		case 'h':
			int childor = fork();
			if (childor < 0)
			{
				exit();
			}
			else if (childor == 0)
			{
				return argc;
			}
			else
			{
				wait(childor);
				if (strcmp(argv[0], "/mkdir"))
				{
					return 0;
				}
				else
				{
					return parsecmd(argv, rightpipe);
				}
			}
			break;
		}
	}

	return argc;
}

void handle_backquotes(char *cmd, char *finish_cmd)
{
	static char sub_cmd_output[1024];

	char *start_bq, *end_bq;
	char *p_read = cmd;
	char *p_write = finish_cmd;

	finish_cmd[0] = '\0';

	while ((start_bq = strchr(p_read, '`')) != NULL)
	{
		end_bq = strchr(start_bq + 1, '`');
		if (end_bq == NULL)
		{
			strcat(p_write, p_read);
			return;
		}

		strncat(p_write, p_read, start_bq - p_read);
		p_write += (start_bq - p_read);

		char sub_cmd[1024];
		strncpy(sub_cmd, start_bq + 1, end_bq - (start_bq + 1));
		sub_cmd[end_bq - (start_bq + 1)] = '\0';

		int p[2];
		pipe(p);
		int r;
		if ((r = fork()) < 0)
		{
			user_panic("fork error");
		}
		if (r == 0)
		{
			dup(p[1], 1);
			close(p[0]);
			close(p[1]);

			runcmd(sub_cmd);
			exit();
		}
		else
		{
			close(p[1]);
			int n;
			int output_num = 0;
			while ((n = read(p[0], sub_cmd_output + output_num, sizeof(sub_cmd_output) - output_num - 1)) > 0)
			{
				output_num += n;
			}
			sub_cmd_output[output_num] = '\0';
			close(p[0]);
			wait(r);
		}
		if (strlen(sub_cmd_output) > 0 && sub_cmd_output[strlen(sub_cmd_output) - 1] == '\n')
		{
			sub_cmd_output[strlen(sub_cmd_output) - 1] = '\0';
		}
		strcat(p_write, sub_cmd_output);
		p_write += strlen(sub_cmd_output);

		p_read = end_bq + 1;
	}
	strcat(p_write, p_read);
}

void runcmd(char *s)
{
	char final_cmd[1024];
	handle_backquotes(s, final_cmd);
	gettoken(final_cmd, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0)
	{
		return;
	}
	argv[argc] = 0;

	if (strcmp(argv[0], "cd") == 0)
	{
		builtin_cd(argc, argv);
		return; // 跳过 fork-runcmd 流程
	}
	if (strcmp(argv[0], "pwd") == 0)
	{
		builtin_pwd(argc, argv);
		return; // 跳过 fork-runcmd 流程
	}
	if (strcmp(argv[0], "exit") == 0)
	{
		builtin_exit(argc, argv); // 直接调用，不会返回
		return;
	}

	// 如果 for 循环结束都没有找到匹配的内置指令，说明是外部命令
	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0)
	{
		wait(child);
	}
	else
	{
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe)
	{
		wait(rightpipe);
	}
	exit();
}

void redraw_line(char *buffer, int cursor_pos, int len, u_int max_display_width)
{
	// 1. 将光标移到行首（当前行的最左边）
	printf("\r");
	// 2. 清除当前行所有内容，打印足够多的空格以覆盖可能的旧内容
	// 打印足够宽的空格来覆盖整行，确保包括提示符后面的所有旧内容都被清除。
	// 这里我们假设 max_display_width 已经包含了提示符的宽度。
	// printf("%*s", max_display_width, " ");
	printf("\x1b[K");
	// 3. 再次将光标移到行首
	printf("\r"); // 确保光标回到最左边，准备打印提示符和命令

	// 4. 重新打印命令内容
	// 注意：这里的打印假设 '$ ' 提示符已在 main 函数或其他地方打印。
	// 如果你希望每次重绘都包含 '$ ' 并且 '$ ' 宽度固定，可以这样：
	printf("$ %s", buffer);
	// 但根据你之前的要求，这里只打印命令内容。
	// printf("%s", buffer);

	// 5. 将光标移动到正确的逻辑位置
	// 如果光标不在命令的逻辑末尾，需要向左移动相应格数
	if (len - cursor_pos > 0)
	{
		printf("\x1b[%dD", len - cursor_pos);
	}
}

void readline(char *buf, u_int n)
{
	if (!iscons(0))
	{
		// 如果不是控制台（例如，是文件重定向），则使用简单的方式读取一行
		int i = 0, r;
		while (i < n - 1)
		{
			// 从标准输入读取一个字符
			r = read(0, buf + i, 1);
			if (r <= 0)
			{ // 如果读到文件末尾或发生错误，则退出
				break;
			}
			// 如果读到换行符，则一行结束
			if (buf[i] == '\n' || buf[i] == '\r')
			{
				break;
			}
			i++;
		}
		buf[i] = 0; // 确保字符串以空字符结尾
		return;		// 读取完毕，直接返回
	}
	int r;
	int len = 0;		// 当前命令的逻辑长度
	int cursor_pos = 0; // 当前光标在 buf 中的逻辑位置 (0到len之间)
	char c_read[3];		// 用于读取输入字符，包括转义序列
	// **更改点：移除 prompt_len，因为 redraw_line 不再处理 $**
	// int prompt_len = 2; // 不再需要

	char current_input[1024];
	memset(current_input, 0, sizeof(current_input));
	// 清空缓冲区并确保null终止
	memset(buf, 0, n);

	while (1)
	{
		r = read(0, c_read, 1);
		if (r != 1)
		{
			if (r < 0)
			{
				debugf("read error: %d\n", r);
			}
			exit();
		}

		char c = c_read[0];

		if (c == 27)
		{						// 转义序列 (ESC)
			read(0, c_read, 1); // 读取 '['
			if (c_read[0] == '[')
			{
				read(0, c_read + 1, 1); // 读取 D, C, A, B 等
				switch (c_read[1])
				{
				case 'D': // 左箭头
					// **更改点：确保 cursor_pos 不会小于 0**
					if (cursor_pos > 0)
					{
						cursor_pos--;
						// 传递 buf 的最大尺寸 n 作为 max_display_width 的近似值
						// 或者你可以定义一个常量，比如 COLUMNS_PER_LINE
						redraw_line(buf, cursor_pos, len, n);
					}
					else
					{
						redraw_line(buf, cursor_pos, len, n);
					}
					break;
				case 'C': // 右箭头
					// **更改点：确保 cursor_pos 不会大于 len**
					if (cursor_pos < len)
					{
						cursor_pos++;
						redraw_line(buf, cursor_pos, len, n);
					}
					else
					{
						redraw_line(buf, cursor_pos, len, n);
					}
					break;
				case 'A': // **更改点：上箭头 (历史)**
					printf("\x1b[B");
					if (history_index > 0)
					{
						// 如果是从当前输入切换到历史，先保存当前输入
						if (history_index == history_count)
						{
							strcpy(current_input, buf);
						}
						history_index--;							  // 向上移动
						strcpy(buf, history_commands[history_index]); // 加载历史命令
						len = strlen(buf);
						cursor_pos = len; // 光标移到行尾
						redraw_line(buf, cursor_pos, len, n);
					}
					else
					{
						// 如果已经是第一条历史指令，再次按上键不切换，但要刷新显示
						redraw_line(buf, cursor_pos, len, n);
					}
					break;
				case 'B': // **更改点：下箭头 (历史)**
					if (history_index < history_count)
					{

						history_index++; // 向下移动
						if (history_index == history_count)
						{								// 切换到当前输入行
							strcpy(buf, current_input); // 恢复之前保存的当前输入
						}
						else
						{
							strcpy(buf, history_commands[history_index]); // 加载历史命令
						}
						len = strlen(buf);
						cursor_pos = len; // 光标移到行尾
						redraw_line(buf, cursor_pos, len, n);
					}
					else
					{
						// 如果已经是最新的输入行，再次按下键不切换，但要刷新显示
						redraw_line(buf, cursor_pos, len, n);
					}
					break;
				}
			}
		}
		else if (c == '\b' || c == 0x7f)
		{ // 退格键
			if (cursor_pos > 0)
			{
				memmove(&buf[cursor_pos - 1], &buf[cursor_pos], len - cursor_pos);
				len--;
				cursor_pos--;
				buf[len] = 0;

				redraw_line(buf, cursor_pos, len, n);
			}
		}
		else if (c == '\r' || c == '\n')
		{ // 回车键
			buf[len] = 0;
			// printf("\n"); // 移动到下一行
			return;
		}
		else if (c == 0x05)
		{ // Ctrl-E (跳到行尾)
			// **更改点：只在光标不在行尾时才需要移动和重绘**
			if (cursor_pos < len)
			{
				cursor_pos = len;
				redraw_line(buf, cursor_pos, len, n);
			}
		}
		else if (c == 0x01)
		{ // Ctrl-A (跳到行首)
			// **更改点：只在光标不在行首时才需要移动和重绘**
			if (cursor_pos > 0)
			{
				cursor_pos = 0;
				// 当光标跳回行首时，需要重新打印 $ 提示符
				// 然后调用 redraw_line 打印命令内容并定位光标
				printf("\r$ ");						  // 先回到行首打印提示符
				redraw_line(buf, cursor_pos, len, n); // 然后重绘命令内容
			}
		}
		else if (c == 0x0B)
		{ // Ctrl-K (删除从光标到行尾)
			if (cursor_pos < len)
			{
				memset(&buf[cursor_pos], 0, len - cursor_pos);
				len = cursor_pos;
				buf[len] = 0;

				redraw_line(buf, cursor_pos, len, n);
			}
		}
		else if (c == 0x15)
		{ // Ctrl-U (删除从行首到光标)
			if (cursor_pos > 0)
			{
				memmove(buf, &buf[cursor_pos], len - cursor_pos);
				len -= cursor_pos;
				memset(&buf[len], 0, cursor_pos);
				cursor_pos = 0;

				redraw_line(buf, cursor_pos, len, n);
			}
		}
		else if (c == 0x17)
		{ // Ctrl-W (向左删除一个单词)
			if (cursor_pos > 0)
			{
				int original_cursor_pos = cursor_pos;
				while (cursor_pos > 0 && isspace(buf[cursor_pos - 1]))
				{
					cursor_pos--;
				}
				while (cursor_pos > 0 && !isspace(buf[cursor_pos - 1]))
				{
					cursor_pos--;
				}

				memmove(&buf[cursor_pos], &buf[original_cursor_pos], len - original_cursor_pos);
				len -= (original_cursor_pos - cursor_pos);
				memset(&buf[len], 0, original_cursor_pos - cursor_pos);
				buf[len] = 0;

				redraw_line(buf, cursor_pos, len, n);
			}
		}
		else if (len < n - 1)
		{ // 普通可打印字符，且缓冲区未满
			memmove(&buf[cursor_pos + 1], &buf[cursor_pos], len - cursor_pos);
			buf[cursor_pos] = c;
			len++;
			cursor_pos++;
			buf[len] = 0;

			redraw_line(buf, cursor_pos, len, n);
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n')
	{
		;
	}
	buf[0] = 0;
}

// **新增：添加指令到历史记录**
void add_to_history(const char *cmd)
{
	// 不保存空指令或只有空白的指令
	if (cmd == NULL || strlen(cmd) == 0)
	{
		return;
	}

	// 如果历史记录已满，移除最旧的指令
	if (history_count >= HISTFILESIZE)
	{
		for (int i = 0; i < HISTFILESIZE - 1; i++)
		{
			strcpy(history_commands[i], history_commands[i + 1]);
		}
		strcpy(history_commands[HISTFILESIZE - 1], cmd);
	}
	else
	{
		strcpy(history_commands[history_count], cmd);
		history_count++;
	}
	history_index = history_count; // 每次添加新命令后，将历史索引重置到末尾
	int fd = open(HISTFILE, O_CREAT | O_WRONLY | O_TRUNC);
	if (fd < 0)
	{
		debugf("history: failed to open %s\n", HISTFILE);
		return;
	}

	// 遍历内存中的所有历史记录并写入文件
	for (int i = 0; i < history_count; i++)
	{
		int len = strlen(history_commands[i]);
		if (write(fd, history_commands[i], len) != len)
		{
			debugf("history: failed to write command\n");
			close(fd);
			return;
		}
		if (write(fd, "\n", 1) != 1)
		{
			debugf("history: failed to write newline\n");
			close(fd);
			return;
		}
	}

	close(fd);
}

int builtin_history(int argc, char **argv)
{
	if (argc > 1)
	{
		printf("history: too many arguments\n");
		return -1;
	}

	int fd;
	char buf[512];
	int n;

	// 打开历史文件
	if ((fd = open(HISTFILE, O_RDONLY)) < 0)
	{
		printf("history: cannot open %s\n", HISTFILE);
		return -1;
	}

	// 循环读取文件内容并打印到标准输出
	while ((n = read(fd, buf, sizeof(buf) - 1)) > 0)
	{
		if (write(1, buf, n) != n)
		{
			printf("history: write error\n");
			close(fd);
			return -1;
		}
	}

	if (n < 0)
	{
		printf("history: read error\n");
	}

	close(fd);
	return 0;
}

int builtin_pwd(int argc, char **argv)
{
	// 1. 严格检查参数数量
	if (argc > 1)
	{
		printf("pwd: expected 0 arguments; got %d\n", argc - 1);
		return 2;
	}

	// 2. 诊断：检查 env 指针本身是否有效
	if (env == NULL)
	{
		printf("pwd: critical error, 'env' pointer is NULL!\n");
		return 1;
	}

	// 3. 诊断：打印 env_cwd 的内存地址和第一个字符的ASCII码
	// 这可以帮助我们确认指针指向的位置和内容的初始部分是否正确
	// printf("Debug: env_cwd address: %x, first char: %d\n", env->env_cwd, (int)env->env_cwd[0]);

	// 4. 核心功能：打印工作目录
	int envid;
	char tem_path[1024] = {0};
	try(syscall_get_cwd(envid, tem_path));
	printf("%s\n", tem_path);
	return 0;
}

int builtin_cd(int argc, char **argv)
{
	if (argc > 2)
	{
		printf("Too many args for cd command\n");
		return 1;
	}

	const char target_path[1024];
	char resolved_path[1024];

	if (argc == 1)
	{
		strcat(target_path, "/");
		resolve_path("/", resolved_path);
	}
	else
	{
		// printf("1\n");
		strcpy(target_path, argv[1]);
		// printf("2\n");
		resolve_path(target_path, resolved_path);
		// printf("2\n");
	}

	// 使用静态缓冲区来接收 stat 的结果，避免栈写入
	struct Stat st;
	int r = stat(resolved_path, &st);
	// printf("3\n");

	if (r < 0)
	{
		printf("cd: The directory '%s' does not exist\n", target_path);
		return 1;
	}
	if (!st.st_isdir)
	{
		printf("cd: '%s' is not a directory\n", target_path);
		return 1;
	}

	// printf("1\n");
	if (*resolved_path == '\0')
	{
		strcat(resolved_path, "/");
	}

	syscall_set_cwd(syscall_getenvid(), resolved_path);
	return 0;
}

int builtin_exit(int argc, char **argv)
{
	// exit 指令不接受任何参数，但我们保持宽容
	exit();	  // 调用系统调用，终止当前进程
	return 0; // 这行实际上不会被执行
}

int builtin_declare(int argc, char **argv)
{
	int x_flag = 0;
	int r_flag = 0;
	ARGBEGIN
	{
	case 'x':
		x_flag = 2;
		break;
	case 'r':
		r_flag = 1;
		break;
	default:;
	}
	ARGEND
	// 遍历参数，解析参数，完成存储
	// 将环境变量存储到env中，通过系统调用实现
	
	return 0;
}

int builtin_unset(int argc, char **argv)
{
	// 通过系统调用完成环境变量的删除
	return 0;
}

char buf[1024];

void usage(void)
{
	printf("usage: sh [-ix] [script-file]\n");
	exit();
}

void init_history()
{
	memset(history_commands, 0, HISTFILESIZE * 1024);
	history_count = 0;
	history_index = 0;
}

int main(int argc, char **argv)
{
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	init_history();
	printf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	printf("::                                                         ::\n");
	printf("::                     MOS Shell 2025                      ::\n");
	printf("::                                                         ::\n");
	printf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN
	{
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1)
	{
		usage();
	}
	if (argc == 1)
	{
		close(0);
		if ((r = open(argv[0], O_RDONLY)) < 0)
		{
			user_panic("open %s: %d", argv[0], r);
		}
		user_assert(r == 0);
	}
	for (;;)
	{
		if (interactive)
		{
			printf("\n$ ");
		}
		readline(buf, sizeof buf);
		if (strlen(buf) > 0)
		{
			add_to_history(buf);
		}
		for (int i = 0; i < strlen(buf); i++)
		{
			if (buf[i] == '#')
			{
				buf[i] = '\0';
				break;
			}
		}

		// 如果命令为空白行或只有注释，parsecmd 可能返回 argc=0
		if (buf[0] == '\0' && !iscons(0))
		{
			exit();
		}
		if (buf[0] == '#')
		{
			continue; // 继续下一个循环
		}
		if (buf[0] == 'c' && buf[1] == 'd')
		{
			// printf("1\n");
			gettoken(buf, 0);

			char *argv[MAXARGS];
			int rightpipe = 0;
			int argc = parsecmd(argv, &rightpipe);
			if (argc == 0)
			{
				exit();
			}
			argv[argc] = 0;
			// printf("1\n");
			builtin_cd(argc, argv);
			continue;
		}

		if (echocmds)
		{
			printf("# %s\n", buf);
		}
		// printf("1\n");
		if ((r = fork()) < 0)
		{
			user_panic("fork: %d", r);
		}
		if (r == 0)
		{
			runcmd(buf);
			exit();
		}
		else
		{
			wait(r);
		}
	}
	return 0;
}

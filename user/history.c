#include <lib.h>
#include <shell.h>

extern char history_commands[HISTFILESIZE][1024];
extern int history_count;

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        printf("history: too many arguments\n");
        return -1;
    }

    for (int i = 0; i < history_count; i++)
    {
        printf("%s\n", history_commands[i]);
    }
    return 0;
}
#include <lib.h>
#include <args.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: touch <file>\n");
        return -1;
    }
    char old_path[1024];
    char path[1024];
    syscall_get_cwd(0, path);
    // printf("%s\n", path);
    strcpy(old_path, argv[1]);
    // printf("%s\n", old_path);
    resolve_path(old_path, path);
    // printf("%s\n", path);

    int fd;
    fd = open(path, O_RDONLY);
    if (fd >= 0)
    {
        return 0;
    }

    fd = open(path, O_CREAT);
    if (fd < 0)
    {
        printf("touch: cannot touch \'%s\': No such file or directory\n", argv[1]);
        return -1;
    }

    close(fd);
    return 0;
}
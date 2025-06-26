#include <lib.h>

int p_flag = 0;

void usage(void)
{
    printf("usage: ls [-dFl] [file...]\n");
    exit();
}

void mkdir(const char *path)
{
    char old_path[1024];
    char newpath[1024];
    syscall_get_cwd(0, newpath);
    // printf("%s\n", path);
    strcpy(old_path, path);
    // printf("%s\n", old_path);
    resolve_path(old_path, newpath);
    // printf("%s\n", path);
    int fd;
    if ((fd = open(newpath, O_RDONLY)) >= 0)
    {
        printf("mkdir: cannot create directory '%s': File exists\n", path);
        close(fd);
        return;
    }
    fd = open(newpath, O_MKDIR);
    if (fd < 0)
    {
        printf("mkdir: cannot create directory '%s': No such file or directory\n", path);
        return;
    }
    close(fd);
}

void mkdir_p(char *path)
{
    char temp_path[1024];
    char *p;
    int i;

    if (path[0] == '\0' || (path[0] == '/' && path[1] == '\0'))
    {
        return;
    }

    for (i = 0; path[i] != '\0' && i < 1023; i++)
    {
        temp_path[i] = path[i];
    }
    temp_path[i] = '\0';

    for (p = temp_path + (temp_path[0] == '/'); *p; p++)
    {
        // printf("%c\n",*(p));
        if (*p == '/')
        {
            *p = '\0';
            // printf("%c\n",*(p-1));
            char old_path[1024];
            char newpath[1024];
            syscall_get_cwd(0, newpath);
            // printf("%s\n", path);
            strcpy(old_path, temp_path);
            // printf("%s\n", old_path);
            resolve_path(old_path, newpath);
            // printf("%s\n", path);

            struct Stat st;
            if (stat(newpath, &st) < 0)
            {
                int fd = open(newpath, O_MKDIR);
                if (fd < 0)
                {
                    printf("mkdir: cannot create directory '%s'", temp_path);
                    return;
                }
                close(fd);
            }
            else if (!st.st_isdir)
            {
                printf("mkdir: cannot create directory '%s': File exists", temp_path);
                return;
            }
            *p = '/';
        }
    }
    char old_path[1024];
    char newpath[1024];
    syscall_get_cwd(0, newpath);
    // printf("%s\n", path);
    strcpy(old_path, temp_path);
    // printf("%s\n", old_path);
    resolve_path(old_path, newpath);
    // printf("%s\n", path);

    struct Stat st;
    if (stat(newpath, &st) < 0)
    {
        int fd = open(newpath, O_MKDIR);
        if (fd < 0)
        {
            printf("mkdir: cannot create directory '%s'", temp_path);
            return;
        }
        close(fd);
    }
    else if (!st.st_isdir)
    {
        printf("mkdir: cannot create directory '%s': File exists", temp_path);
        return;
    }
}

int main(int argc, char **argv)
{

    ARGBEGIN
    {
    default:
        usage();
    case 'p':
        p_flag = 1;
        break;
    }
    ARGEND

    if (p_flag)
    {
        mkdir_p(argv[0]);
    }
    else
    {
        mkdir(argv[0]);
    }
    return 0;
}
#include <lib.h>
#include <args.h>

int r_flag = 0;
int f_flag = 0;

void usage(void)
{
    printf("usage: rm [-rf] [file...]\n");
    exit();
}

void recursive_rm(const char *path) {
    struct Stat st;
    int r;

    char old_path[1024];
    char newpath[1024];
    syscall_get_cwd(0, newpath);
    //printf("%s\n", path);
    strcpy(old_path, path);
    //printf("%s\n", old_path);
    resolve_path(old_path, newpath);
    //printf("%s\n", path);
    
    // 1. 获取路径信息
    r = stat(newpath, &st);
    if (r < 0) {
        if (!f_flag) { // 如果没有-f，则报告错误
            printf("rm: cannot remove '%s': No such file or directory\n", path);
        }
        return;
    }
    
    // 2. 如果是文件，直接删除
    if (!st.st_isdir) {
        remove(newpath);
        return;
    }

    // 3. 如果是目录，但没有 -r 选项，则报错
    if (!r_flag) {
        printf("rm: cannot remove '%s': Is a directory\n", path);
        return;
    }
    
    // 4. 递归删除目录内容
    int fd = open(newpath, O_RDONLY);
    if (fd < 0) {
        if (!f_flag) printf("rm: cannot open directory '%s'\n", path);
        return;
    }

    char buf[512];
    int n;
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        struct File *f = (struct File*)buf;
        if (strcmp(f->f_name, ".") != 0 && strcmp(f->f_name, "..") != 0) {
            char child_path[MAXPATHLEN];
            strcpy(child_path, path);
            strcat(child_path, "/");
            strcat(child_path, f->f_name);
            recursive_rm(child_path);
        }
    }
    close(fd);

    // 5. 删除空目录本身
    remove(newpath);
}

int main(int argc, char **argv) {
    int i;
    ARGBEGIN {
        case 'r':
            r_flag = 1;
            break;
        case 'f':
            f_flag = 1;
            break;
        default:
            usage();
            return 1;
    }
    ARGEND
    
    // -rf 是 -r 和 -f 的组合
    if (f_flag) r_flag = 1;

    if (argc == 0) {
        printf("Usage: rm [-rf] <file...>\n");
        return 1;
    }

    for (i = 0; i < argc; i++) {
        recursive_rm(argv[i]);
    }

    return 0;
}
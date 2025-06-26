#include <lib.h>

char buf[8192];

void cat(int f, char *s)
{
	long n;
	int r;

	while ((n = read(f, buf, (long)sizeof buf)) > 0)
	{
		if ((r = write(1, buf, n)) != n)
		{
			user_panic("write error copying %s: %d", s, r);
		}
	}
	if (n < 0)
	{
		user_panic("error reading %s: %d", s, n);
	}
}

int main(int argc, char **argv)
{
	int f, i;

	if (argc == 1)
	{
		cat(0, "<stdin>");
	}
	else
	{
		for (i = 1; i < argc; i++)
		{
			char old_path[1024];
			char path[1024];
			syscall_get_cwd(0, path);
			// printf("%s\n", path);
			strcpy(old_path, argv[i]);
			// printf("%s\n", old_path);
			resolve_path(old_path, path);
			// printf("%s\n", path);
			f = open(path, O_RDONLY);
			if (f < 0)
			{
				user_panic("can't open %s: %d", argv[i], f);
			}
			else
			{
				cat(f, argv[i]);
				close(f);
			}
		}
	}
	return 0;
}

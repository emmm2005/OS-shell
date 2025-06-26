#include <types.h>

void *memcpy(void *dst, const void *src, size_t n)
{
	void *dstaddr = dst;
	void *max = dst + n;

	if (((u_long)src & 3) != ((u_long)dst & 3))
	{
		while (dst < max)
		{
			*(char *)dst++ = *(char *)src++;
		}
		return dstaddr;
	}

	while (((u_long)dst & 3) && dst < max)
	{
		*(char *)dst++ = *(char *)src++;
	}

	// copy machine words while possible
	while (dst + 4 <= max)
	{
		*(uint32_t *)dst = *(uint32_t *)src;
		dst += 4;
		src += 4;
	}

	// finish the remaining 0-3 bytes
	while (dst < max)
	{
		*(char *)dst++ = *(char *)src++;
	}
	return dstaddr;
}

void *memset(void *dst, int c, size_t n)
{
	void *dstaddr = dst;
	void *max = dst + n;
	u_char byte = c & 0xff;
	uint32_t word = byte | byte << 8 | byte << 16 | byte << 24;

	while (((u_long)dst & 3) && dst < max)
	{
		*(u_char *)dst++ = byte;
	}

	// fill machine words while possible
	while (dst + 4 <= max)
	{
		*(uint32_t *)dst = word;
		dst += 4;
	}

	// finish the remaining 0-3 bytes
	while (dst < max)
	{
		*(u_char *)dst++ = byte;
	}
	return dstaddr;
}

size_t strlen(const char *s)
{
	int n;

	for (n = 0; *s; s++)
	{
		n++;
	}

	return n;
}

char *strcpy(char *dst, const char *src)
{
	char *ret = dst;

	while ((*dst++ = *src++) != 0)
	{
	}

	return ret;
}

char *strncpy(char *dst, const char *src, int n)
{
	char *ret = dst;

	while (n && (*dst++ = *src++) != 0)
	{
		n--;
	}
	*dst = '\0';
	return ret;
}

const char *strchr(const char *s, int c)
{
	for (; *s; s++)
	{
		if (*s == c)
		{
			return s;
		}
	}
	return 0;
}

int strcmp(const char *p, const char *q)
{
	while (*p && *p == *q)
	{
		p++, q++;
	}

	if ((u_int)*p < (u_int)*q)
	{
		return -1;
	}

	if ((u_int)*p > (u_int)*q)
	{
		return 1;
	}

	return 0;
}

char *strcat(char *dest, const char *src)
{
	// 是否需要检查指针是否为空
	char *temp = dest;
	while (*temp != '\0')
	{ // 定位到 dest 的末尾
		temp++;
	}
	while ((*temp++ = *src++) != '\0')
		; // 将 src 的内容复制到 dest 的末尾

	return dest; // 返回 dest 的起始地址
}

char *strncat(char *dest, const char *src, int n)
{
	// 是否需要检查指针是否为空
	char *temp = dest;
	while (*temp != '\0')
	{ // 定位到 dest 的末尾
		temp++;
	}
	while (n && (*temp++ = *src++) != '\0')
		n--; // 将 src 的内容复制到 dest 的末尾

	*temp = '\0';
	return dest; // 返回 dest 的起始地址
}

void *memmove(void *dest, const void *src, size_t n)
{
	unsigned char *d = (unsigned char *)dest;
	const unsigned char *s = (const unsigned char *)src;

	if (d < s)
	{
		// 目标地址在源地址之前，从前往后复制
		// 示例： src: [A B C D E], dest: [  A B C D E]
		// 结果：   dest: [A B C D E]
		while (n--)
		{
			*d++ = *s++;
		}
	}
	else
	{
		// 目标地址在源地址之后或重叠，从后往前复制
		// 示例： src: [A B C D E], dest: [X X A B C] (重叠)
		// 结果：   dest: [X X A B C]
		//           src: [A B C D E]
		d += n;
		s += n;
		while (n--)
		{
			*--d = *--s;
		}
	}
	return dest;
}

int isspace(int c)
{
	// 检查字符是否为以下任意一种空白字符
	// 空格 (space)
	// 换页 (form feed)
	// 换行 (new line)
	// 回车 (carriage return)
	// 水平制表符 (horizontal tab)
	// 垂直制表符 (vertical tab)
	return (c == ' ' ||
			c == '\f' ||
			c == '\n' ||
			c == '\r' ||
			c == '\t' ||
			c == '\v');
}

size_t strspn(const char *s, const char *accept) {
    const char *p;
    const char *a;
    size_t count = 0;

    for (p = s; *p != '\0'; ++p) {
        for (a = accept; *a != '\0'; ++a) {
            if (*p == *a) {
                break;
            }
        }
        if (*a == '\0') {
            return count;
        }
        ++count;
    }

    return count;
}

size_t strcspn(const char *s, const char *reject) {
    const char *p;
    const char *r;
    size_t count = 0;

    for (p = s; *p != '\0'; ++p) {
        for (r = reject; *r != '\0'; ++r) {
            if (*p == *r) {
                return count;
            }
        }
        ++count;
    }

    return count;
}

static char *last_token = NULL;

char *strtok(char *str, const char *delim) {
    char *token;

    // 如果 str 不为 NULL，表示开始一次新的切分。
    if (str != NULL) {
        last_token = str;
    }

    // 如果 last_token 为 NULL，表示上一次切分已经结束。
    if (last_token == NULL) {
        return NULL;
    }

    // 1. 跳过所有前导的分隔符。
    last_token += strspn(last_token, delim);
    if (*last_token == '\0') {
        // 如果已经到了字符串末尾，说明没有 token 了。
        last_token = NULL;
        return NULL;
    }

    // 2. 找到了 token 的开头，现在寻找 token 的结尾。
    token = last_token;
    last_token += strcspn(last_token, delim);

    // 3. 如果我们还没到字符串的末尾，
    //    就在 token 的结尾处放置一个 '\0'，并更新 last_token 以便下次调用。
    if (*last_token != '\0') {
        *last_token = '\0';
        last_token++;
    } else {
        // 如果已经到了字符串的末尾，下次调用应该返回 NULL。
        last_token = NULL;
    }

    return token;
}

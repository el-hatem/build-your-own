#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "shell/xalloc.h"

void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (!p)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

void *xcalloc(size_t count, size_t size)
{
    void *p = calloc(count, size);
    if (!p)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

char *xstrdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *p = xmalloc(len);
    memcpy(p, s, len);
    return p;
}
#ifndef SHELL_XALLOC_H
#define SHELL_XALLOC_H

#include <stddef.h>

void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
char *xstrdup(const char *s);

#endif
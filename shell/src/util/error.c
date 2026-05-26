#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "shell/error.h"

void shell_perror(const char *ctx)
{
    fprintf(stderr, "%s: %s\n", ctx, strerror(errno));
}

void shell_error(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}
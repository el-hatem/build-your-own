#include <stdio.h>
#include "util/log.h"

void log_debug(const char *msg)
{
    fprintf(stderr, "debug: %s\n", msg);
}

void log_info(const char *msg)
{
    fprintf(stderr, "info: %s\n", msg);
}
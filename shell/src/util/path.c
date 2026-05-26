#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util/path.h"

char *path_join(const char *a, const char *b)
{
    size_t la = strlen(a);
    size_t lb = strlen(b);
    char *out = malloc(la + lb + 2);
    if (!out)
        return NULL;
    memcpy(out, a, la);
    out[la] = '/';
    memcpy(out + la + 1, b, lb + 1);
    return out;
}

int path_is_executable(const char *path)
{
    return access(path, X_OK) == 0;
}
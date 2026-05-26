#include <string.h>

#include "expand/quote_removal.h"
#include "shell/xalloc.h"

char *quote_remove(const char *input)
{
    return xstrdup(input);
}
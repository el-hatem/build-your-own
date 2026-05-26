#include <glob.h>
#include <stdlib.h>
#include <string.h>
#include "expand/globbing.h"

int expand_glob_word(const char *pattern, char ***outv)
{
    glob_t g;
    size_t i;

    if (glob(pattern, 0, NULL, &g) != 0)
        return 1;
    *outv = calloc(g.gl_pathc + 1, sizeof(char *));
    if (!*outv)
    {
        globfree(&g);
        return 1;
    }
    for (i = 0; i < g.gl_pathc; i++)
        (*outv)[i] = strdup(g.gl_pathv[i]);
    globfree(&g);
    return 0;
}
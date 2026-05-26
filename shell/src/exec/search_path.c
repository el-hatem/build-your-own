#include <stdlib.h>
#include <string.h>
#include "exec/search_path.h"
#include "expand/variables.h"
#include "util/path.h"

char *search_in_path(struct s_shell *sh, const char *name)
{
    const char *path = shell_get_var(sh, "PATH");
    char *dup;
    char *save;
    char *tok;
    char *full;

    if (!path || strchr(name, '/'))
        return strdup(name);
    dup = strdup(path);
    if (!dup)
        return NULL;
    save = NULL;
    tok = strtok_r(dup, ":", &save);
    while (tok)
    {
        full = path_join(tok, name);
        if (full && path_is_executable(full))
        {
            free(dup);
            return full;
        }
        free(full);
        tok = strtok_r(NULL, ":", &save);
    }
    free(dup);
    return NULL;
}
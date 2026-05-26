#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "builtins/type.h"
#include "builtins/builtin_registry.h"
#include "exec/search_path.h"
#include "util/path.h"

static int print_type_for_name(struct s_shell *sh, const char *name)
{
    const t_builtin_spec *spec;
    char *path;

    spec = builtin_find(sh, name);
    if (spec)
    {
        printf("%s is a shell builtin\n", name);
        return 0;
    }
    path = search_in_path(sh, name);
    if (path && path_is_executable(path))
    {
        printf("%s is %s\n", name, path);
        free(path);
        return 0;
    }
    free(path);
    printf("%s not found\n", name);
    return 1;
}

int bi_type(struct s_shell *sh, char **argv)
{
    int status = 0;
    int i = 1;

    if (!argv[1])
        return 0;
    while (argv[i])
    {
        if (print_type_for_name(sh, argv[i]))
            status = 1;
        i++;
    }
    return status;
}
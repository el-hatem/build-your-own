#include <string.h>
#include "shell/shell.h"
#include "builtins/builtins.h"
#include "builtins/builtin_registry.h"

int builtin_dispatch(struct s_shell *sh, char **argv)
{
    const t_builtin_spec *spec;

    if (!argv || !argv[0])
        return -1;
    spec = builtin_find(sh, argv[0]);
    if (!spec || !spec->fn)
        return -1;
    sh->last_status = spec->fn(sh, argv);
    return sh->last_status;
}

int builtin_is_parent_only(const char *name)
{
    return (!strcmp(name, "cd") || !strcmp(name, "exit") || !strcmp(name, "export")
        || !strcmp(name, "declare") || !strcmp(name, "jobs") || !strcmp(name, "history")
        || !strcmp(name, "complete"));
}
#include <stddef.h>
#include <string.h>
#include "shell/shell.h"
#include "builtins/builtin_registry.h"
#include "builtins/cd.h"
#include "builtins/exit.h"
#include "builtins/pwd.h"
#include "builtins/echo.h"
#include "builtins/export.h"
#include "builtins/declare.h"
#include "builtins/jobs.h"
#include "builtins/history.h"
#include "builtins/complete.h"
#include "builtins/type.h"

static t_builtin_spec g_defaults[] = {
    {"cd", bi_cd, true},
    {"exit", bi_exit, true},
    {"pwd", bi_pwd, false},
    {"echo", bi_echo, false},
    {"export", bi_export, true},
    {"declare", bi_declare, true},
    {"jobs", bi_jobs, true},
    {"history", bi_history, true},
    {"complete", bi_complete, true},
    {"type", bi_type, false},
    {NULL, NULL, false}
};

int builtin_register_defaults(struct s_shell *sh)
{
    sh->builtins.items = g_defaults;
    sh->builtins.count = 10;
    return 0;
}

const t_builtin_spec *builtin_find(struct s_shell *sh, const char *name)
{
    size_t i = 0;

    while (i < sh->builtins.count)
    {
        if (strcmp(sh->builtins.items[i].name, name) == 0)
            return &sh->builtins.items[i];
        ++i;
    }
    return NULL;
}
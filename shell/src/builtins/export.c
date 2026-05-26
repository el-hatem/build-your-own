#include <string.h>
#include <stdio.h>
#include "builtins/export.h"
#include "expand/variables.h"

int bi_export(struct s_shell *sh, char **argv)
{
    char *eq;
    int i = 1;

    if (!argv[1])
        return 0;
    while (argv[i])
    {
        eq = strchr(argv[i], '=');
        if (!eq)
        {
            fprintf(stderr, "export: invalid format: %s\n", argv[i]);
            return 1;
        }
        *eq = '\0';
        if (shell_set_exported(sh, argv[i], eq + 1) != 0)
        {
            *eq = '=';
            return 1;
        }
        *eq = '=';
        i++;
    }
    return 0;
}
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "builtins/cd.h"
#include "shell/shell.h"
#include "expand/variables.h"

static char *expand_cd_target(struct s_shell *sh, const char *target)
{
    const char *home;

    if (!target || target[0] != '~')
        return strdup(target);
    home = shell_get_var(sh, "HOME");
    if (!home)
        return NULL;
    if (target[1] == '\0')
        return strdup(home);
    if (target[1] == '/')
    {
        size_t home_len = strlen(home);
        size_t tail_len = strlen(target + 1);
        char *expanded = malloc(home_len + tail_len + 1);

        if (!expanded)
            return NULL;
        memcpy(expanded, home, home_len);
        memcpy(expanded + home_len, target + 1, tail_len + 1);
        return expanded;
    }
    return strdup(target);
}

int bi_cd(struct s_shell *sh, char **argv)
{
    const char *target = argv[1] ? argv[1] : shell_get_var(sh, "HOME");
    char *expanded;

    if (!target)
        return 1;
    expanded = expand_cd_target(sh, target);
    if (!expanded)
        return 1;
    if (chdir(expanded) != 0)
    {
        fprintf(stderr, "cd: %s: %s\n", expanded, strerror(errno));
        free(expanded);
        return 1;
    }
    free(expanded);
    return 0;
}
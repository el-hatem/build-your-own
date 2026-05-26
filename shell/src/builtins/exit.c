#include <stdlib.h>
#include "builtins/exit.h"
#include "shell/shell.h"

int bi_exit(struct s_shell *sh, char **argv)
{
    if (argv[1])
        sh->last_status = atoi(argv[1]);
    sh->should_exit = true;
    return sh->last_status;
}
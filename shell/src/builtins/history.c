#include "builtins/history.h"
#include "input/history.h"
#include "shell/shell.h"

int bi_history(struct s_shell *sh, char **argv)
{
    return history_command(sh, argv);
}
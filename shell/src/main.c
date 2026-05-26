#include "shell/shell.h"
#include <stdio.h>

int main(int argc, char **argv, char **envp)
{
    t_shell sh;

    (void)argc;
    (void)argv;
    if (shell_init(&sh, envp) != 0)
        return 1;
    shell_run(&sh);
    shell_destroy(&sh);
    return sh.last_status;
}
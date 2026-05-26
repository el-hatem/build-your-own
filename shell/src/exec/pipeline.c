#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "exec/pipeline.h"
#include "exec/redir.h"
#include "exec/search_path.h"
#include "builtins/builtins.h"
#include "ast/ast.h"
#include "shell/shell.h"
#include "expand/variables.h"

int exec_pipeline(struct s_shell *sh, t_ast *node, struct s_exec_ctx *ctx)
{
    size_t i;
    int pipes[2 * 32];
    pid_t pids[32];
    size_t n;
    char **envp;
    int status;

    (void)ctx;
    n = node->as.pipeline.count;
    if (n == 0 || n > 32)
        return 1;
    for (i = 0; i + 1 < n; i++)
    {
        if (pipe(&pipes[i * 2]) < 0)
            return 1;
    }
    envp = shell_build_envp(sh);
    for (i = 0; i < n; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            size_t j;
            char **argv;
            char *path;
            int rc;

            if (i > 0)
                dup2(pipes[(i - 1) * 2], STDIN_FILENO);
            if (i + 1 < n)
                dup2(pipes[i * 2 + 1], STDOUT_FILENO);
            for (j = 0; j < 2 * (n - 1); j++)
                close(pipes[j]);
            apply_redirections(node->as.pipeline.items[i]->as.simple.redirs);
            argv = ast_simple_argv_dup(&node->as.pipeline.items[i]->as.simple);
            if (!argv[0])
                _exit(0);
            rc = builtin_dispatch(sh, argv);
            if (rc >= 0)
                _exit(rc);
            path = search_in_path(sh, argv[0]);
            execve(path ? path : argv[0], argv, envp);
            _exit(127);
        }
    }
    for (i = 0; i < 2 * (n - 1); i++)
        close(pipes[i]);
    for (i = 0; i < n; i++)
        waitpid(pids[i], &status, 0);
    if (WIFEXITED(status))
        sh->last_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
        sh->last_status = 128 + WTERMSIG(status);
    return sh->last_status;
}
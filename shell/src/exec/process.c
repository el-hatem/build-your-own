#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "exec/process.h"
#include "exec/search_path.h"
#include "expand/variables.h"
#include "jobs/job_table.h"
#include "shell/error.h"
#include "shell/shell.h"

char *resolve_executable(const char *cmd, char *const envp[])
{
    (void)envp;
    return strdup(cmd);
}

pid_t spawn_external(struct s_shell *sh, char *const argv[], int background)
{
    pid_t pid;
    char *path;
    char **envp;
    int status;

    path = search_in_path(sh, argv[0]);
    if (!path)
    {
        fprintf(stderr, "%s: command not found\n", argv[0]);
        sh->last_status = 127;
        return -1;
    }
    envp = shell_build_envp(sh);
    pid = fork();
    if (pid == 0)
    {
        execve(path, argv, envp);
        if (errno == ENOENT)
            fprintf(stderr, "%s: command not found\n", argv[0]);
        else
            perror(argv[0]);
        _exit(127);
    }
    free(path);
    if (background)
    {
        job_table_add(&sh->jobs, pid, argv);
        return pid;
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        sh->last_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
        sh->last_status = 128 + WTERMSIG(status);
    return pid;
}
#ifndef SHELL_SHELL_H
#define SHELL_SHELL_H

#include <stdbool.h>
#include <sys/types.h>
#include <stddef.h>


#include "jobs/jobs.h"
#include "input/history.h"
#include "builtins/builtin_registry.h"

typedef struct s_env_entry {
    char *name;
    char *value;
} t_env_entry;

typedef struct s_env {
    t_env_entry *exported;
    t_env_entry *locals;
    size_t       exported_len;
    size_t       exported_cap;
    size_t       locals_len;
    size_t       locals_cap;
} t_env;

typedef struct s_shell {
    t_env                env;
    t_jobs               jobs;
    t_history            history;
    int                  last_status;
    bool                 interactive;
    bool                 should_exit;
    int                  exit_requested;
    pid_t                shell_pgid;
    int                  tty_fd;
    char                *history_file;
    t_builtin_registry   builtins;
} t_shell;

int  shell_init(t_shell *sh, char **envp);
int  shell_run(t_shell *sh);
void shell_destroy(t_shell *sh);

#endif
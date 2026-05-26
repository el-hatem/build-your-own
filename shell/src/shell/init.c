#include <string.h>
#include <unistd.h>
#include "shell/shell.h"
#include "input/readline_wrap.h"
#include "input/history.h"
#include "input/completion.h"
#include "builtins/builtin_registry.h"
#include "jobs/jobs.h"
#include "jobs/signals.h"
#include "shell/config.h"
#include "expand/variables.h"

static const char *shell_history_path(t_shell *sh)
{
    const char *path;

    path = shell_get_var(sh, "HISTFILE");
    if (path && *path)
        return path;
    return SHELL_HISTORY_DEFAULT;
}

int shell_init(t_shell *sh, char **envp)
{
    memset(sh, 0, sizeof(*sh));
    sh->interactive = isatty(STDIN_FILENO);
    sh->last_status = 0;
    if (shell_init_env(sh, envp) != 0)
        return 1;
    sh->history_file = (char *)shell_history_path(sh);
    if (jobs_init(sh) != 0)
        return 1;
    if (signals_init(sh) != 0)
        return 1;
    if (readline_wrap_init(sh) != 0)
        return 1;
    if (completion_init(sh) != 0)
        return 1;
    if (builtin_register_defaults(sh) != 0)
        return 1;
    if (history_init(sh, sh->history_file) != 0)
        return 1;
    history_load(sh);
    return 0;
}
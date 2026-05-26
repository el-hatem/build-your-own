#include <stdlib.h>
#include "shell/shell.h"
#include "input/history.h"
#include "jobs/jobs.h"
#include "expand/variables.h"
#include "builtins/complete.h"

void shell_destroy(t_shell *sh)
{
    history_destroy(sh);
    jobs_destroy(sh);
    shell_destroy_env(sh);
    bi_complete_destroy();
}
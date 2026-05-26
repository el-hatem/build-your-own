#include <stdio.h>
#include "jobs/jobs.h"
#include "builtins/jobs.h"
#include "shell/shell.h"

static char jobs_marker_for_index(const t_jobs *jobs, size_t index)
{
    if (jobs->len == 0)
        return ' ';
    if (index == jobs->len - 1)
        return '+';
    if (jobs->len >= 2 && index == jobs->len - 2)
        return '-';
    return ' ';
}

static const char *jobs_state_name(t_job_state state)
{
    if (state == JOB_RUNNING)
        return "Running";
    if (state == JOB_STOPPED)
        return "Stopped";
    return "Done";
}

static void print_job_command(const t_job *job)
{
    if (job->state == JOB_DONE)
    {
        size_t len = 0;

        while (job->command[len] != '\0')
            len++;
        if (len >= 2 && job->command[len - 2] == ' ' && job->command[len - 1] == '&')
            printf("%.*s", (int)(len - 2), job->command);
        else
            printf("%s", job->command);
        return;
    }
    printf("%s", job->command);
    if (job->state == JOB_RUNNING)
        printf(" &");
}

int bi_jobs(struct s_shell *sh, char **argv)
{
    size_t i;

    (void)argv;
    jobs_mark_done(sh);
    for (i = 0; i < sh->jobs.len; i++)
    {
        printf("[%d]%c  %-24s ",
            sh->jobs.items[i].id,
            jobs_marker_for_index(&sh->jobs, i),
            jobs_state_name(sh->jobs.items[i].state));
        print_job_command(&sh->jobs.items[i]);
        printf("\n");
    }
    jobs_reap(sh);
    return 0;
}
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "shell/shell.h"
#include "jobs/jobs.h"
#include "jobs/signals.h"

static const char *job_state_name(t_job_state state)
{
    if (state == JOB_RUNNING)
        return "Running";
    if (state == JOB_STOPPED)
        return "Stopped";
    return "Done";
}

static char job_marker_for_index(const t_jobs *jobs, size_t index)
{
    ssize_t current;
    ssize_t previous;

    current = jobs->len > 0 ? (ssize_t)(jobs->len - 1) : -1;
    previous = jobs->len > 1 ? (ssize_t)(jobs->len - 2) : -1;
    if ((ssize_t)index == current)
        return '+';
    if ((ssize_t)index == previous)
        return '-';
    return ' ';
}

static void print_done_command(const t_job *job)
{
    size_t len;

    len = strlen(job->command);
    if (len >= 2 && strcmp(job->command + len - 2, " &") == 0)
        printf("%.*s", (int)(len - 2), job->command);
    else
        printf("%s", job->command);
}

int jobs_init(struct s_shell *sh)
{
    return job_table_init(&sh->jobs);
}

int jobs_mark_done(struct s_shell *sh)
{
    int status;
    pid_t pid;

    g_sigchld_received = 0;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        job_table_mark_done(&sh->jobs, pid, status);
    return 0;
}

int jobs_reap_completed(struct s_shell *sh)
{
    size_t i;

    jobs_mark_done(sh);
    i = 0;
    while (i < sh->jobs.len)
    {
        if (sh->jobs.items[i].state == JOB_DONE)
        {
            printf("[%d]%c  %-24s ",
                sh->jobs.items[i].id,
                job_marker_for_index(&sh->jobs, i),
                job_state_name(sh->jobs.items[i].state));
            print_done_command(&sh->jobs.items[i]);
            printf("\n");
        }
        i++;
    }
    job_table_reap(&sh->jobs);
    return 0;
}

int jobs_reap(struct s_shell *sh)
{
    job_table_reap(&sh->jobs);
    return 0;
}

void jobs_destroy(struct s_shell *sh)
{
    job_table_destroy(&sh->jobs);
}
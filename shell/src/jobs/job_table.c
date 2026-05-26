#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "jobs/job_table.h"
#include "shell/xalloc.h"

static char *join_command(char *const argv[])
{
    size_t i;
    size_t len;
    char *command;
    size_t pos;

    len = 0;
    i = 0;
    while (argv[i])
    {
        len += strlen(argv[i]);
        if (argv[i + 1])
            len++;
        i++;
    }
    command = malloc(len + 1);
    if (!command)
        return NULL;
    pos = 0;
    i = 0;
    while (argv[i])
    {
        size_t arg_len = strlen(argv[i]);

        memcpy(command + pos, argv[i], arg_len);
        pos += arg_len;
        if (argv[i + 1])
            command[pos++] = ' ';
        i++;
    }
    command[pos] = '\0';
    return command;
}

static int next_available_job_id(const t_jobs *jobs)
{
    int candidate;
    size_t i;
    int used;

    candidate = 1;
    while (1)
    {
        used = 0;
        i = 0;
        while (i < jobs->len)
        {
            if (jobs->items[i].id == candidate)
            {
                used = 1;
                break;
            }
            i++;
        }
        if (!used)
            return candidate;
        candidate++;
    }
}

t_job *job_table_find_by_pgid(t_jobs *jobs, pid_t pgid)
{
    size_t i;

    for (i = 0; i < jobs->len; i++)
    {
        if (jobs->items[i].pgid == pgid)
            return &jobs->items[i];
    }
    return NULL;
}

int job_table_init(t_jobs *jobs)
{
    jobs->items = NULL;
    jobs->len = 0;
    jobs->cap = 0;
    jobs->next_id = 1;
    return 0;
}

int job_table_add(t_jobs *jobs, pid_t pgid, char *const argv[])
{
    t_job *new_items;
    char *command;
    int job_id;

    if (jobs->len + 1 > jobs->cap)
    {
        jobs->cap = jobs->cap ? jobs->cap * 2 : 8;
        new_items = realloc(jobs->items, jobs->cap * sizeof(*new_items));
        if (!new_items)
            return 1;
        jobs->items = new_items;
    }
    command = join_command(argv);
    if (!command)
        return 1;
    job_id = next_available_job_id(jobs);
    jobs->items[jobs->len].id = job_id;
    jobs->items[jobs->len].pgid = pgid;
    jobs->items[jobs->len].command = command;
    jobs->items[jobs->len].state = JOB_RUNNING;
    jobs->items[jobs->len].status = 0;
    jobs->len++;
    fprintf(stderr, "[%d] %d\n", jobs->items[jobs->len - 1].id, (int)pgid);
    return 0;
}

void job_table_mark_done(t_jobs *jobs, pid_t pgid, int status)
{
    t_job *job;

    job = job_table_find_by_pgid(jobs, pgid);
    if (job)
    {
        job->state = JOB_DONE;
        job->status = status;
    }
}

void job_table_reap(t_jobs *jobs)
{
    size_t i;
    size_t j;

    i = 0;
    j = 0;
    while (i < jobs->len)
    {
        if (jobs->items[i].state == JOB_DONE)
            free(jobs->items[i].command);
        else
            jobs->items[j++] = jobs->items[i];
        i++;
    }
    jobs->len = j;
}

void job_table_destroy(t_jobs *jobs)
{
    size_t i;

    for (i = 0; i < jobs->len; i++)
        free(jobs->items[i].command);
    free(jobs->items);
    jobs->items = NULL;
    jobs->len = 0;
    jobs->cap = 0;
}
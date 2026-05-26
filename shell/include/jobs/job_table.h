#ifndef JOBS_JOB_TABLE_H
#define JOBS_JOB_TABLE_H

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum e_job_state {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} t_job_state;

typedef struct s_job {
    int         id;
    pid_t       pgid;
    char       *command;
    t_job_state state;
    int         status;
} t_job;

typedef struct s_jobs {
    t_job  *items;
    size_t  len;
    size_t  cap;
    int     next_id;
} t_jobs;

int   job_table_init(t_jobs *jobs);
int   job_table_add(t_jobs *jobs, pid_t pgid, char *const argv[]);
void  job_table_mark_done(t_jobs *jobs, pid_t pgid, int status);
void  job_table_reap(t_jobs *jobs);
void  job_table_destroy(t_jobs *jobs);
t_job *job_table_find_by_pgid(t_jobs *jobs, pid_t pgid);

#endif
#ifndef JOBS_JOBS_H
#define JOBS_JOBS_H

#include "jobs/job_table.h"

struct s_shell;
int jobs_init(struct s_shell *sh);
int jobs_mark_done(struct s_shell *sh);
int jobs_reap_completed(struct s_shell *sh);
int jobs_reap(struct s_shell *sh);
void jobs_destroy(struct s_shell *sh);

#endif
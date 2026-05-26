#ifndef JOBS_PROCESS_GROUP_H
#define JOBS_PROCESS_GROUP_H

#include <sys/types.h>

int process_group_set(pid_t pid, pid_t pgid);

#endif
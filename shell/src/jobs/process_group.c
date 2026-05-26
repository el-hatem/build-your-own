#include <unistd.h>
#include "jobs/process_group.h"

int process_group_set(pid_t pid, pid_t pgid)
{
    return setpgid(pid, pgid);
}
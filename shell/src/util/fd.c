#include <unistd.h>
#include "util/fd.h"

int fd_close_quiet(int fd)
{
    if (fd >= 0)
        return close(fd);
    return 0;
}

int fd_dup2_checked(int oldfd, int newfd)
{
    return dup2(oldfd, newfd);
}
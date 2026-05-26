#include <unistd.h>
#include <fcntl.h>
#include "exec/redir.h"

int apply_redirections(t_redir *redir)
{
    int fd;

    while (redir)
    {
        if (redir->type == R_IN)
            fd = open(redir->target.text, O_RDONLY);
        else if (redir->type == R_OUT || redir->type == R_ERR_OUT)
            fd = open(redir->target.text, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        else
            fd = open(redir->target.text, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0)
            return 1;
        if (dup2(fd, redir->fd) < 0)
        {
            close(fd);
            return 1;
        }
        close(fd);
        redir = redir->next;
    }
    return 0;
}

int save_stdio(int saved[3])
{
    saved[0] = dup(STDIN_FILENO);
    saved[1] = dup(STDOUT_FILENO);
    saved[2] = dup(STDERR_FILENO);
    return (saved[0] < 0 || saved[1] < 0 || saved[2] < 0);
}

int restore_stdio(int saved[3])
{
    int rc = 0;

    rc |= dup2(saved[0], STDIN_FILENO) < 0;
    rc |= dup2(saved[1], STDOUT_FILENO) < 0;
    rc |= dup2(saved[2], STDERR_FILENO) < 0;
    close(saved[0]);
    close(saved[1]);
    close(saved[2]);
    return rc;
}
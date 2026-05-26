#ifndef UTIL_FD_H
#define UTIL_FD_H

int fd_close_quiet(int fd);
int fd_dup2_checked(int oldfd, int newfd);

#endif
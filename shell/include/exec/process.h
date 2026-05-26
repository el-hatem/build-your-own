#ifndef EXEC_PROCESS_H
#define EXEC_PROCESS_H

#include <sys/types.h>

struct s_shell;
char *resolve_executable(const char *cmd, char *const envp[]);
pid_t spawn_external(struct s_shell *sh, char *const argv[], int background);

#endif
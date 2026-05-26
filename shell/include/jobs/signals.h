#ifndef JOBS_SIGNALS_H
#define JOBS_SIGNALS_H

#include <signal.h>
struct s_shell;
int  signals_init(struct s_shell *sh);
void signals_set_interactive_handlers(void);
extern volatile sig_atomic_t g_sigchld_received;

#endif
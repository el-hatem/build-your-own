#include <signal.h>
#include <string.h>
#include "jobs/signals.h"

volatile sig_atomic_t g_sigchld_received = 0;

static void sigchld_handler(int signo)
{
    (void)signo;
    g_sigchld_received = 1;
}

int signals_init(struct s_shell *sh)
{
    struct sigaction sa;
    (void)sh;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
        return 1;
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    return 0;
}

void signals_set_interactive_handlers(void)
{
}
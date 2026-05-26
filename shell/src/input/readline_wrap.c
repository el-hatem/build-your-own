#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "input/readline_wrap.h"

int readline_wrap_init(struct s_shell *sh)
{
    (void)sh;
    using_history();
    return 0;
}

char *readline_wrap_read(const char *prompt)
{
    return readline(prompt);
}
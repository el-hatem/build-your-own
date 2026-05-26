#include <stdio.h>
#include <unistd.h>
#include "builtins/pwd.h"

int bi_pwd(struct s_shell *sh, char **argv)
{
    char buf[4096];
    (void)sh;
    (void)argv;
    if (!getcwd(buf, sizeof(buf)))
        return 1;
    printf("%s\n", buf);
    return 0;
}
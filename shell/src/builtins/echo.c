#include <stdio.h>
#include <string.h>
#include "builtins/echo.h"

int bi_echo(struct s_shell *sh, char **argv)
{
    int i = 1;
    int newline = 1;
    (void)sh;

    if (argv[1] && strcmp(argv[1], "-n") == 0)
    {
        newline = 0;
        i = 2;
    }
    while (argv && argv[i])
    {
        if (i > (newline ? 1 : 2))
            printf(" ");
        printf("%s", argv[i]);
        i++;
    }
    if (newline)
        printf("\n");
    return 0;
}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/history.h>
#include "input/history.h"
#include "shell/shell.h"
#include "shell/xalloc.h"

static int history_append_since_load(struct s_shell *sh, const char *path)
{
    int new_entries;
    int result;

    new_entries = history_length - (int)sh->history.loaded;
    if (new_entries < 0)
        new_entries = 0;
    result = append_history(new_entries, path);
    if (result == 0)
        sh->history.loaded = history_length;
    return result;
}

static int history_start_index(char **argv, int total)
{
    int start;
    int n;
    char *endptr;

    start = 0;
    if (argv[1] && argv[2] == NULL)
    {
        n = (int)strtol(argv[1], &endptr, 10);
        if (*argv[1] != '\0' && *endptr == '\0' && n > 0 && n < total)
            start = total - n;
    }
    return start;
}

static int history_print(struct s_shell *sh, char **argv)
{
    HIST_ENTRY **list;
    int total;
    int base;
    int start;
    int i;

    (void)sh;
    list = history_list();
    if (!list)
        return 0;
    total = history_length;
    base = history_base;
    start = history_start_index(argv, total);
    for (i = start; i < total; i++)
    {
        printf("%5d  %s\n", base + i, list[i]->line);
    }
    return 0;
}

int history_init(struct s_shell *sh, const char *path)
{
    sh->history.path = xstrdup(path);
    sh->history.loaded = 0;
    return 0;
}

int history_load(struct s_shell *sh)
{
    if (read_history(sh->history.path) == 0)
        sh->history.loaded = history_length;
    return 0;
}

int history_save(struct s_shell *sh, int append_mode)
{
    int result;

    if (append_mode)
        return history_append_since_load(sh, sh->history.path);
    result = write_history(sh->history.path);
    if (result == 0)
        sh->history.loaded = history_length;
    return result;
}

int history_add_entry(struct s_shell *sh, const char *line)
{
    (void)sh;
    if (line && *line)
        add_history(line);
    return 0;
}

void history_destroy(struct s_shell *sh)
{
    free(sh->history.path);
    sh->history.path = NULL;
}

int history_command(struct s_shell *sh, char **argv)
{
    int result;

    if (argv[1] && argv[2] && argv[3] == NULL)
    {
        if (strcmp(argv[1], "-r") == 0)
            return read_history(argv[2]);
        if (strcmp(argv[1], "-w") == 0)
        {
            result = write_history(argv[2]);
            if (result == 0)
                sh->history.loaded = history_length;
            return result;
        }
        if (strcmp(argv[1], "-a") == 0)
            return history_append_since_load(sh, argv[2]);
    }
    return history_print(sh, argv);
}
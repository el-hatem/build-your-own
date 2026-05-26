#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "builtins/complete.h"

typedef struct s_completion_spec {
    char *command;
    char *script;
    struct s_completion_spec *next;
} t_completion_spec;

static t_completion_spec *g_specs;

static char *dup_string(const char *value)
{
    char *copy;

    copy = malloc(strlen(value) + 1);
    if (!copy)
        return NULL;
    strcpy(copy, value);
    return copy;
}

static t_completion_spec *find_spec(const char *command)
{
    t_completion_spec *spec;

    spec = g_specs;
    while (spec)
    {
        if (strcmp(spec->command, command) == 0)
            return spec;
        spec = spec->next;
    }
    return NULL;
}

const char *complete_get_script(const char *command)
{
    const t_completion_spec *spec;

    spec = find_spec(command);
    if (!spec)
        return NULL;
    return spec->script;
}

int complete_unregister(const char *command)
{
    t_completion_spec *spec;
    t_completion_spec *prev;

    prev = NULL;
    spec = g_specs;
    while (spec)
    {
        if (strcmp(spec->command, command) == 0)
        {
            if (prev)
                prev->next = spec->next;
            else
                g_specs = spec->next;
            free(spec->command);
            free(spec->script);
            free(spec);
            return 0;
        }
        prev = spec;
        spec = spec->next;
    }
    return 1;
}

static void complete_unregister_all(void)
{
    t_completion_spec *spec;
    t_completion_spec *next;

    spec = g_specs;
    while (spec)
    {
        next = spec->next;
        free(spec->command);
        free(spec->script);
        free(spec);
        spec = next;
    }
    g_specs = NULL;
}

static int register_spec(const char *command, const char *script)
{
    t_completion_spec *spec;
    char *command_copy;
    char *script_copy;

    spec = find_spec(command);
    if (spec)
    {
        script_copy = dup_string(script);
        if (!script_copy)
            return 1;
        free(spec->script);
        spec->script = script_copy;
        return 0;
    }
    command_copy = dup_string(command);
    script_copy = dup_string(script);
    if (!command_copy || !script_copy)
    {
        free(command_copy);
        free(script_copy);
        return 1;
    }
    spec = malloc(sizeof(*spec));
    if (!spec)
    {
        free(command_copy);
        free(script_copy);
        return 1;
    }
    spec->command = command_copy;
    spec->script = script_copy;
    spec->next = g_specs;
    g_specs = spec;
    return 0;
}

static void print_spec(const t_completion_spec *spec, const char *command)
{
    if (!spec)
    {
        fprintf(stderr, "complete: %s: no completion specification\n", command);
        return;
    }
    printf("complete -C '%s' %s\n", spec->script, spec->command);
}

static int bi_complete_print_spec(char **argv)
{
    const t_completion_spec *spec;

    if (!argv[2])
        return 1;
    spec = find_spec(argv[2]);
    print_spec(spec, argv[2]);
    return spec ? 0 : 1;
}

static int bi_complete_register_spec(char **argv)
{
    if (!argv[2] || !argv[3])
        return 1;
    return register_spec(argv[3], argv[2]);
}

static int bi_complete_unregister_spec(char **argv)
{
    if (!argv[2])
    {
        complete_unregister_all();
        return 0;
    }
    return complete_unregister(argv[2]);
}

int bi_complete(struct s_shell *sh, char **argv)
{
    (void)sh;
    if (argv[1] && strcmp(argv[1], "-p") == 0)
        return bi_complete_print_spec(argv);
    if (argv[1] && strcmp(argv[1], "-C") == 0)
        return bi_complete_register_spec(argv);
    if (argv[1] && strcmp(argv[1], "-r") == 0)
        return bi_complete_unregister_spec(argv);
    return 0;
}

void bi_complete_destroy(void)
{
    t_completion_spec *spec;
    t_completion_spec *next;

    spec = g_specs;
    while (spec)
    {
        next = spec->next;
        free(spec->command);
        free(spec->script);
        free(spec);
        spec = next;
    }
    g_specs = NULL;
}
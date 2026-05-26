#include <string.h>
#include <stdio.h>
#include "builtins/declare.h"
#include "expand/variables.h"

static int declare_valid_name(const char *name)
{
    size_t i;

    if (!name || !name[0] || (!(name[0] >= 'A' && name[0] <= 'Z')
        && !(name[0] >= 'a' && name[0] <= 'z') && name[0] != '_'))
        return 0;
    i = 1;
    while (name[i])
    {
        if (!(name[i] >= 'A' && name[i] <= 'Z')
            && !(name[i] >= 'a' && name[i] <= 'z')
            && !(name[i] >= '0' && name[i] <= '9') && name[i] != '_')
            return 0;
        i++;
    }
    return 1;
}

static int declare_print_one(struct s_shell *sh, const char *name)
{
    const char *value;

    value = shell_get_var(sh, name);
    if (!value)
    {
        fprintf(stderr, "declare: %s: not found\n", name);
        return 1;
    }
    printf("declare -- %s=\"%s\"\n", name, value);
    return 0;
}

static int declare_print(struct s_shell *sh, char **argv)
{
    int i;

    i = 2;
    if (!argv[i])
        return 0;
    while (argv[i])
    {
        if (declare_print_one(sh, argv[i]) != 0)
            return 1;
        i++;
    }
    return 0;
}

static int declare_assign(struct s_shell *sh, char **argv)
{
    char *eq;
    int i;

    i = 1;
    while (argv[i])
    {
        eq = strchr(argv[i], '=');
        if (!eq)
        {
            fprintf(stderr, "declare: invalid format: %s\n", argv[i]);
            return 1;
        }
        *eq = '\0';
        if (!declare_valid_name(argv[i]))
        {
            *eq = '=';
            fprintf(stderr, "declare: `%s': not a valid identifier\n", argv[i]);
            return 1;
        }
        *eq = '\0';
        if (shell_set_local(sh, argv[i], eq + 1) != 0)
        {
            *eq = '=';
            return 1;
        }
        *eq = '=';
        i++;
    }
    return 0;
}

int bi_declare(struct s_shell *sh, char **argv)
{
    if (!argv[1])
        return 0;
    if (strcmp(argv[1], "-p") == 0)
        return declare_print(sh, argv);
    return declare_assign(sh, argv);
}
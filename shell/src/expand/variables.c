#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "expand/variables.h"
#include "shell/shell.h"
#include "shell/xalloc.h"

static int valid_name(const char *name)
{
    size_t i = 0;
    if (!name || !name[0] || (!(isalpha((unsigned char)name[0]) || name[0] == '_')))
        return 0;
    while (name[++i])
        if (!(isalnum((unsigned char)name[i]) || name[i] == '_'))
            return 0;
    return 1;
}

static int env_set(t_env_entry **arr, size_t *len, size_t *cap, const char *name, const char *value)
{
    size_t i;
    t_env_entry *new_arr;

    for (i = 0; i < *len; i++)
    {
        if (strcmp((*arr)[i].name, name) == 0)
        {
            free((*arr)[i].value);
            (*arr)[i].value = xstrdup(value ? value : "");
            return 0;
        }
    }
    if (*len + 1 > *cap)
    {
        *cap = *cap ? *cap * 2 : 16;
        new_arr = realloc(*arr, *cap * sizeof(**arr));
        if (!new_arr)
            return 1;
        *arr = new_arr;
    }
    (*arr)[*len].name = xstrdup(name);
    (*arr)[*len].value = xstrdup(value ? value : "");
    (*len)++;
    return 0;
}

static const char *env_get_arr(t_env_entry *arr, size_t len, const char *name)
{
    size_t i;
    for (i = 0; i < len; i++)
        if (strcmp(arr[i].name, name) == 0)
            return arr[i].value;
    return NULL;
}

int shell_init_env(struct s_shell *sh, char **envp)
{
    char *eq;
    size_t i = 0;

    while (envp && envp[i])
    {
        eq = strchr(envp[i], '=');
        if (eq)
        {
            *eq = '\0';
            env_set(&sh->env.exported, &sh->env.exported_len, &sh->env.exported_cap, envp[i], eq + 1);
            *eq = '=';
        }
        i++;
    }
    return 0;
}

void shell_destroy_env(struct s_shell *sh)
{
    size_t i;

    for (i = 0; i < sh->env.exported_len; i++)
    {
        free(sh->env.exported[i].name);
        free(sh->env.exported[i].value);
    }
    for (i = 0; i < sh->env.locals_len; i++)
    {
        free(sh->env.locals[i].name);
        free(sh->env.locals[i].value);
    }
    free(sh->env.exported);
    free(sh->env.locals);
}

int shell_set_local(struct s_shell *sh, const char *name, const char *value)
{
    if (!valid_name(name))
        return 1;
    return env_set(&sh->env.locals, &sh->env.locals_len, &sh->env.locals_cap, name, value);
}

int shell_set_exported(struct s_shell *sh, const char *name, const char *value)
{
    if (!valid_name(name))
        return 1;
    return env_set(&sh->env.exported, &sh->env.exported_len, &sh->env.exported_cap, name, value);
}

const char *shell_get_var(struct s_shell *sh, const char *name)
{
    const char *v = env_get_arr(sh->env.locals, sh->env.locals_len, name);
    if (v)
        return v;
    return env_get_arr(sh->env.exported, sh->env.exported_len, name);
}

static int append_char(char **buf, size_t *len, size_t *cap, char c)
{
    char *newbuf;
    size_t newcap;

    if (*len + 1 >= *cap)
    {
        newcap = *cap ? (*cap * 2) : 32;
        newbuf = realloc(*buf, newcap);
        if (!newbuf)
            return 1;
        *buf = newbuf;
        *cap = newcap;
    }
    (*buf)[(*len)++] = c;
    (*buf)[*len] = '\0';
    return 0;
}

static int append_str(char **buf, size_t *len, size_t *cap, const char *s)
{
    while (*s)
        if (append_char(buf, len, cap, *s++))
            return 1;
    return 0;
}

char *expand_variables(struct s_shell *sh, const char *input, unsigned char quoted)
{
    char *out = NULL;
    size_t len = 0, cap = 0, i = 0, j;
    char name[256];
    const char *val;
    (void)quoted;

    while (input[i])
    {
        if (input[i] == '$')
        {
            if (input[i + 1] == '?')
            {
                char num[32];
                snprintf(num, sizeof(num), "%d", sh->last_status);
                if (append_str(&out, &len, &cap, num))
                    return NULL;
                i += 2;
                continue;
            }
            if (input[i + 1] == '{')
            {
                i += 2;
                j = 0;
                while (input[i] && input[i] != '}' && j + 1 < sizeof(name))
                    name[j++] = input[i++];
                name[j] = '\0';
                if (input[i] == '}')
                    i++;
                val = shell_get_var(sh, name);
                if (val && append_str(&out, &len, &cap, val))
                    return NULL;
                continue;
            }
            j = 0;
            i++;
            while (input[i] && (isalnum((unsigned char)input[i]) || input[i] == '_') && j + 1 < sizeof(name))
                name[j++] = input[i++];
            name[j] = '\0';
            val = shell_get_var(sh, name);
            if (val && append_str(&out, &len, &cap, val))
                return NULL;
            continue;
        }
        if (append_char(&out, &len, &cap, input[i++]))
            return NULL;
    }
    if (!out)
        out = xstrdup("");
    return out;
}

char **shell_build_envp(struct s_shell *sh)
{
    char **envp = xcalloc(sh->env.exported_len + 1, sizeof(*envp));
    size_t i;

    for (i = 0; i < sh->env.exported_len; i++)
    {
        size_t n = strlen(sh->env.exported[i].name);
        size_t v = strlen(sh->env.exported[i].value);
        envp[i] = xmalloc(n + v + 2);
        memcpy(envp[i], sh->env.exported[i].name, n);
        envp[i][n] = '=';
        memcpy(envp[i] + n + 1, sh->env.exported[i].value, v + 1);
    }
    return envp;
}
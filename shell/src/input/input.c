#include <stdlib.h>
#include <string.h>

#include "input/input.h"
#include "input/readline_wrap.h"
#include "shell/shell.h"

static int quote_step_escape(const char *line, size_t *i, char in_quote)
{
    if (line[*i] == '\\' && in_quote != '\'' && line[*i + 1])
    {
        *i += 2;
        return 1;
    }
    return 0;
}

static int quote_step_toggle(const char *line, size_t *i, char *in_quote)
{
    if (line[*i] == '\'' && *in_quote != '"')
    {
        *in_quote = (*in_quote == '\'') ? 0 : '\'';
        (*i)++;
        return 1;
    }
    if (line[*i] == '"' && *in_quote != '\'')
    {
        *in_quote = (*in_quote == '"') ? 0 : '"';
        (*i)++;
        return 1;
    }
    return 0;
}

static int line_has_unclosed_quote(const char *line)
{
    char in_quote = 0;
    size_t i = 0;

    while (line[i])
    {
        if (quote_step_escape(line, &i, in_quote))
            continue;
        if (quote_step_toggle(line, &i, &in_quote))
            continue;
        i++;
    }
    return in_quote != 0;
}

static char *join_lines(char *first, char *second)
{
    size_t first_len;
    size_t second_len;
    char *joined;

    first_len = strlen(first);
    second_len = strlen(second);
    joined = malloc(first_len + second_len + 1);
    if (!joined)
        return NULL;
    memcpy(joined, first, first_len);
    memcpy(joined + first_len, second, second_len + 1);
    free(first);
    free(second);
    return joined;
}

char *input_read_line(struct s_shell *sh, const char *prompt)
{
    char *line;
    char *chunk;

    (void)sh;
    line = readline_wrap_read(prompt);
    if (!line)
        return NULL;
    while (sh && sh->interactive && line_has_unclosed_quote(line))
    {
        chunk = readline_wrap_read("> ");
        if (!chunk)
            break;
        chunk = join_lines(line, chunk);
        if (!chunk)
        {
            free(line);
            return NULL;
        }
        line = chunk;
    }
    return line;
}
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer/lexer.h"
#include "parser/parse_error.h"
#include "shell/xalloc.h"

typedef struct s_op_spec {
    const char *lexeme;
    t_token_type type;
    size_t length;
} t_op_spec;

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

int token_stream_push(t_token_stream *ts, t_token tok)
{
    t_token *new_items;
    size_t new_cap;

    if (ts->len + 1 >= ts->cap)
    {
        new_cap = ts->cap ? ts->cap * 2 : 16;
        new_items = realloc(ts->items, new_cap * sizeof(*new_items));
        if (!new_items)
            return 1;
        ts->items = new_items;
        ts->cap = new_cap;
    }
    ts->items[ts->len++] = tok;
    return 0;
}

static int push_op(t_token_stream *out, t_token_type type, const char *lex, size_t pos)
{
    t_token tok;

    tok.type = type;
    tok.lexeme = xstrdup(lex);
    tok.pos = pos;
    tok.quoted = 0;
    return token_stream_push(out, tok);
}

static int lex_is_op_break(char c)
{
    return (c == '|' || c == '&' || c == ';' || c == '<' || c == '>');
}

static int lex_toggle_quote(char c, char *in_quote, unsigned char *quoted, size_t *i)
{
    if (c == '\'' && *in_quote != '"')
    {
        *quoted = 1;
        *in_quote = (*in_quote == '\'') ? 0 : '\'';
        (*i)++;
        return 1;
    }
    if (c == '"' && *in_quote != '\'')
    {
        *quoted = 1;
        *in_quote = (*in_quote == '"') ? 0 : '"';
        (*i)++;
        return 1;
    }
    return 0;
}

static int lex_consume_escape(const char *input, size_t *i, char in_quote, char **buf, size_t *len, size_t *cap)
{
    if (input[*i] != '\\' || in_quote == '\'')
        return 0;
    if (input[*i + 1] == '\n')
    {
        *i += 2;
        return 1;
    }
    if (input[*i + 1])
    {
        if (append_char(buf, len, cap, input[*i + 1]))
            return -1;
        *i += 2;
        return 1;
    }
    return 0;
}

static int lex_consume_special(const char *input, size_t *i, char *in_quote, unsigned char *quoted,
    char **buf, size_t *len, size_t *cap)
{
    int rc;

    rc = lex_consume_escape(input, i, *in_quote, buf, len, cap);
    if (rc != 0)
        return rc;
    if (lex_toggle_quote(input[*i], in_quote, quoted, i))
        return 1;
    return 0;
}

static int lex_word_step(const char *input, size_t *i, char *in_quote, unsigned char *quoted,
    char **buf, size_t *len, size_t *cap)
{
    int rc;

    if (!*in_quote && (isspace((unsigned char)input[*i]) || lex_is_op_break(input[*i])))
        return 1;
    rc = lex_consume_special(input, i, in_quote, quoted, buf, len, cap);
    if (rc != 0)
        return (rc < 0) ? -1 : 2;
    if (append_char(buf, len, cap, input[*i]))
        return -1;
    (*i)++;
    return 0;
}

static int lex_word(const char *input, size_t *i, t_token_stream *out)
{
    char *buf = NULL;
    size_t len = 0, cap = 0, pos = *i;
    unsigned char quoted = 0;
    char in_quote = 0;
    int rc;

    while (input[*i])
    {
        rc = lex_word_step(input, i, &in_quote, &quoted, &buf, &len, &cap);
        if (rc < 0)
            return free(buf), 1;
        if (rc == 1)
            break;
        if (rc == 2)
            continue;
    }
    if (in_quote)
    {
        parse_error_unexpected("EOF");
        return free(buf), 1;
    }
    return token_stream_push(out, (t_token){TOK_WORD, buf ? buf : xstrdup(""), pos, quoted});
}

static int lex_operator(const char *input, size_t *i, t_token_stream *out)
{
    static const t_op_spec ops[] = {
        {"||", TOK_OR_IF, 2},
        {"&&", TOK_AND_IF, 2},
        {"2>>", TOK_REDIR_ERR_APP, 3},
        {"2>", TOK_REDIR_ERR, 2},
        {">>", TOK_REDIR_APPEND, 2},
        {"|", TOK_PIPE, 1},
        {"&", TOK_AMP, 1},
        {";", TOK_SEMI, 1},
        {"<", TOK_REDIR_IN, 1},
        {">", TOK_REDIR_OUT, 1},
    };
    size_t j;

    for (j = 0; j < sizeof(ops) / sizeof(ops[0]); j++)
    {
        if (strncmp(&input[*i], ops[j].lexeme, ops[j].length) == 0)
        {
            if (push_op(out, ops[j].type, ops[j].lexeme, *i))
                return 1;
            *i += ops[j].length;
            return 1;
        }
    }
    return 0;
}

static int lex_fd1_redirect(const char *input, size_t *i, t_token_stream *out)
{
    if (input[*i] != '1' || input[*i + 1] != '>')
        return 0;
    if (input[*i + 2] == '>')
    {
        if (push_op(out, TOK_REDIR_APPEND, ">>", *i))
            return 1;
        *i += 3;
        return 1;
    }
    if (push_op(out, TOK_REDIR_OUT, ">", *i))
        return 1;
    *i += 2;
    return 1;
}

int lexer_tokenize(const char *input, t_token_stream *out)
{
    size_t i = 0;

    out->items = NULL;
    out->len = 0;
    out->cap = 0;
    out->cursor = 0;
    while (input[i])
    {
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }
        if (input[i] == '#')
            break;
        if (lex_fd1_redirect(input, &i, out))
            continue;
        if (lex_operator(input, &i, out))
            continue;
        if (lex_word(input, &i, out))
            return 1;
    }
    return push_op(out, TOK_EOF, "", i);
}

void token_stream_free(t_token_stream *ts)
{
    size_t i = 0;

    while (i < ts->len)
    {
        free(ts->items[i].lexeme);
        i++;
    }
    free(ts->items);
    ts->items = NULL;
    ts->len = 0;
    ts->cap = 0;
    ts->cursor = 0;
}
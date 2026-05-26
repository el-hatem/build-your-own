#ifndef LEXER_TOKEN_H
#define LEXER_TOKEN_H

#include <stddef.h>

typedef enum e_token_type {
    TOK_WORD,
    TOK_PIPE,
    TOK_AND_IF,
    TOK_OR_IF,
    TOK_AMP,
    TOK_SEMI,
    TOK_REDIR_IN,
    TOK_REDIR_OUT,
    TOK_REDIR_APPEND,
    TOK_REDIR_ERR,
    TOK_REDIR_ERR_APP,
    TOK_EOF,
    TOK_ERROR
} t_token_type;

typedef struct s_token {
    t_token_type type;
    char        *lexeme;
    size_t       pos;
    unsigned char quoted;
} t_token;

typedef struct s_token_stream {
    t_token *items;
    size_t   len;
    size_t   cap;
    size_t   cursor;
} t_token_stream;

int  token_stream_push(t_token_stream *ts, t_token tok);
void token_stream_free(t_token_stream *ts);

#endif
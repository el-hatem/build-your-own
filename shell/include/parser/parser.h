#ifndef PARSER_PARSER_H
#define PARSER_PARSER_H

#include "lexer/token.h"
#include "ast/ast.h"

typedef struct s_parser {
    t_token_stream *ts;
} t_parser;

int parser_init(t_parser *p, t_token_stream *ts);
int parser_parse(t_parser *p, t_ast **out_ast);

#endif
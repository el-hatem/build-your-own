#ifndef PARSER_GRAMMAR_H
#define PARSER_GRAMMAR_H

#include "parser/parser.h"

int parse_list(t_parser *p, t_ast **out);
int parse_pipeline(t_parser *p, t_ast **out);
int parse_simple_command(t_parser *p, t_ast **out);

#endif
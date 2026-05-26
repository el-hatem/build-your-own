#include "parser/parser.h"
#include "parser/grammar.h"

int parser_init(t_parser *p, t_token_stream *ts)
{
    p->ts = ts;
    return 0;
}

int parser_parse(t_parser *p, t_ast **out_ast)
{
    return parse_list(p, out_ast);
}
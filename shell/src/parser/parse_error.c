#include <stdio.h>
#include "parser/parse_error.h"

void parse_error_unexpected(const char *near_tok)
{
    fprintf(stderr, "parse error near `%s`\n", near_tok ? near_tok : "EOF");
}
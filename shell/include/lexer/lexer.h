#ifndef LEXER_LEXER_H
#define LEXER_LEXER_H

#include "lexer/token.h"

int lexer_tokenize(const char *input, t_token_stream *out);

#endif
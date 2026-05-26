#include <ctype.h>
#include "lexer/scan.h"

int scan_is_operator_char(int c)
{
    return (c == '|' || c == '&' || c == ';' || c == '<' || c == '>');
}

int scan_is_word_char(int c)
{
    return !scan_is_operator_char(c) && !isspace((unsigned char)c);
}
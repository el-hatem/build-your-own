#ifndef EXEC_REDIR_H
#define EXEC_REDIR_H

#include "ast/ast.h"

int apply_redirections(t_redir *redir);
int save_stdio(int saved[3]);
int restore_stdio(int saved[3]);

#endif
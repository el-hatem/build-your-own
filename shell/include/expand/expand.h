#ifndef EXPAND_EXPAND_H
#define EXPAND_EXPAND_H

struct s_shell;
struct s_ast;

int expand_ast(struct s_shell *sh, struct s_ast *ast);

#endif
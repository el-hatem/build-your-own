#ifndef EXEC_EXEC_H
#define EXEC_EXEC_H

#include "ast/ast.h"
#include "exec/exec_ctx.h"

struct s_shell;

int exec_ast(struct s_shell *sh, t_ast *node, t_exec_ctx *ctx);
int exec_simple(struct s_shell *sh, t_simple_cmd *cmd, t_exec_ctx *ctx);

#endif
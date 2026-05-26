#ifndef EXEC_PIPELINE_H
#define EXEC_PIPELINE_H

#include "ast/ast.h"

struct s_shell;
struct s_exec_ctx;

int exec_pipeline(struct s_shell *sh, t_ast *node, struct s_exec_ctx *ctx);

#endif
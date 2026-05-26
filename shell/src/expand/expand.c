#include <stdlib.h>
#include "expand/expand.h"
#include "expand/quote_removal.h"
#include "expand/variables.h"
#include "ast/ast.h"

static int expand_simple(struct s_shell *sh, t_simple_cmd *cmd)
{
    size_t i;
    char *expanded;
    t_redir *r;

    for (i = 0; i < cmd->argc; i++)
    {
        expanded = expand_variables(sh, cmd->argv[i].text, cmd->argv[i].quoted);
        if (!expanded)
            return 1;
        free(cmd->argv[i].text);
        cmd->argv[i].text = quote_remove(expanded);
        free(expanded);
    }
    r = cmd->redirs;
    while (r)
    {
        expanded = expand_variables(sh, r->target.text, r->target.quoted);
        if (!expanded)
            return 1;
        free(r->target.text);
        r->target.text = quote_remove(expanded);
        free(expanded);
        r = r->next;
    }
    return 0;
}

int expand_ast(struct s_shell *sh, struct s_ast *ast)
{
    size_t i;

    if (!ast)
        return 0;
    if (ast->type == AST_SIMPLE || ast->type == AST_BACKGROUND)
        return expand_simple(sh, &ast->as.simple);
    if (ast->type == AST_PIPELINE)
    {
        for (i = 0; i < ast->as.pipeline.count; i++)
            if (expand_ast(sh, ast->as.pipeline.items[i]))
                return 1;
        return 0;
    }
    return expand_ast(sh, ast->as.binary.left) || expand_ast(sh, ast->as.binary.right);
}
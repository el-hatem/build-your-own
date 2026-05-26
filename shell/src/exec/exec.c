#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "exec/exec.h"
#include "exec/pipeline.h"
#include "exec/redir.h"
#include "exec/process.h"
#include "builtins/builtins.h"
#include "ast/ast.h"
#include "shell/shell.h"
#include "expand/variables.h"

static int is_assignment_only(t_simple_cmd *cmd)
{
    size_t i;
    char *eq;

    if (cmd->argc == 0)
        return 0;
    for (i = 0; i < cmd->argc; i++)
    {
        eq = strchr(cmd->argv[i].text, '=');
        if (!eq || eq == cmd->argv[i].text)
            return 0;
    }
    return 1;
}

static int apply_simple_redirs(t_simple_cmd *cmd, int saved[3])
{
    if (!cmd->redirs)
        return 0;
    if (save_stdio(saved))
        return 1;
    if (apply_redirections(cmd->redirs))
    {
        restore_stdio(saved);
        return 1;
    }
    return 0;
}

static void restore_simple_redirs(t_simple_cmd *cmd, int saved[3])
{
    if (cmd->redirs)
    {
        fflush(stdout);
        fflush(stderr);
        restore_stdio(saved);
    }
}

static int run_simple_command(struct s_shell *sh, char **argv, t_exec_ctx *ctx)
{
    if (builtin_is_parent_only(argv[0]))
        return builtin_dispatch(sh, argv);
    if (builtin_dispatch(sh, argv) < 0)
        return spawn_external(sh, argv, ctx && ctx->background);
    return sh->last_status;
}

static int exec_ast_binary(struct s_shell *sh, t_ast *node, t_exec_ctx *ctx)
{
    int rc;

    if (node->type == AST_LIST)
    {
        exec_ast(sh, node->as.binary.left, ctx);
        return exec_ast(sh, node->as.binary.right, ctx);
    }
    if (node->type == AST_AND)
    {
        rc = exec_ast(sh, node->as.binary.left, ctx);
        if (rc == 0)
            rc = exec_ast(sh, node->as.binary.right, ctx);
        return rc;
    }
    rc = exec_ast(sh, node->as.binary.left, ctx);
    if (rc != 0)
        rc = exec_ast(sh, node->as.binary.right, ctx);
    return rc;
}

int exec_simple(struct s_shell *sh, t_simple_cmd *cmd, t_exec_ctx *ctx)
{
    char **argv;
    int saved[3];
    size_t i;
    char *eq;

    if (is_assignment_only(cmd))
    {
        for (i = 0; i < cmd->argc; i++)
        {
            eq = strchr(cmd->argv[i].text, '=');
            *eq = '\0';
            shell_set_local(sh, cmd->argv[i].text, eq + 1);
            *eq = '=';
        }
        return 0;
    }
    argv = ast_simple_argv_dup(cmd);
    if (apply_simple_redirs(cmd, saved))
    {
        free(argv);
        return 1;
    }
    if (!argv[0])
    {
        restore_simple_redirs(cmd, saved);
        free(argv);
        return 0;
    }
    run_simple_command(sh, argv, ctx);
    restore_simple_redirs(cmd, saved);
    for (i = 0; argv[i]; i++)
        free(argv[i]);
    free(argv);
    return sh->last_status;
}

int exec_ast(struct s_shell *sh, t_ast *node, t_exec_ctx *ctx)
{
    if (!node)
        return 0;
    if (node->type == AST_PIPELINE)
        return exec_pipeline(sh, node, ctx);
    if (node->type == AST_SIMPLE)
        return exec_simple(sh, &node->as.simple, ctx);
    if (node->type == AST_BACKGROUND)
    {
        t_exec_ctx bg = *ctx;

        bg.background = true;
        return exec_simple(sh, &node->as.simple, &bg);
    }
    if (node->type == AST_LIST || node->type == AST_AND || node->type == AST_OR)
        return exec_ast_binary(sh, node, ctx);
    return 0;
}
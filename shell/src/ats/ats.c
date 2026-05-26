#include <stdlib.h>
#include <string.h>
#include "ast/ast.h"
#include "shell/xalloc.h"

static void free_redirs(t_redir *r)
{
    t_redir *next;
    while (r)
    {
        next = r->next;
        free(r->target.text);
        free(r);
        r = next;
    }
}

t_ast *ast_new_simple(void)
{
    t_ast *node = xcalloc(1, sizeof(*node));
    node->type = AST_SIMPLE;
    return node;
}

t_ast *ast_new_pipeline(void)
{
    t_ast *node = xcalloc(1, sizeof(*node));
    node->type = AST_PIPELINE;
    return node;
}

t_ast *ast_new_binary(t_ast_type type, t_ast *left, t_ast *right)
{
    t_ast *node = xcalloc(1, sizeof(*node));
    node->type = type;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

int ast_simple_add_arg(t_ast *node, const char *text, unsigned char quoted)
{
    t_word *newv = realloc(node->as.simple.argv, (node->as.simple.argc + 1) * sizeof(*newv));
    if (!newv)
        return 1;
    node->as.simple.argv = newv;
    node->as.simple.argv[node->as.simple.argc].text = xstrdup(text);
    node->as.simple.argv[node->as.simple.argc].quoted = quoted;
    node->as.simple.argc++;
    return 0;
}

int ast_simple_add_redir(t_ast *node, t_redir_type type, int fd, const char *target, unsigned char quoted)
{
    t_redir *r = xcalloc(1, sizeof(*r));
    t_redir **tail = &node->as.simple.redirs;

    r->type = type;
    r->fd = fd;
    r->target.text = xstrdup(target);
    r->target.quoted = quoted;
    while (*tail)
        tail = &(*tail)->next;
    *tail = r;
    return 0;
}

int ast_pipeline_add(t_ast *node, t_ast *child)
{
    t_ast **new_items = realloc(node->as.pipeline.items, (node->as.pipeline.count + 1) * sizeof(*new_items));
    if (!new_items)
        return 1;
    node->as.pipeline.items = new_items;
    node->as.pipeline.items[node->as.pipeline.count++] = child;
    return 0;
}

char **ast_simple_argv_dup(const t_simple_cmd *cmd)
{
    char **argv;
    size_t i;
    size_t j;
    size_t kept;

    kept = 0;
    for (i = 0; i < cmd->argc; i++)
    {
        if (cmd->argv[i].text[0] != '\0' || cmd->argv[i].quoted)
            kept++;
    }
    argv = xcalloc(kept + 1, sizeof(*argv));
    j = 0;
    for (i = 0; i < cmd->argc; i++)
    {
        if (cmd->argv[i].text[0] == '\0' && !cmd->argv[i].quoted)
            continue;
        argv[j++] = xstrdup(cmd->argv[i].text);
    }
    return argv;
}

void ast_free(t_ast *node)
{
    size_t i;

    if (!node)
        return;
    if (node->type == AST_SIMPLE || node->type == AST_BACKGROUND)
    {
        for (i = 0; i < node->as.simple.argc; i++)
            free(node->as.simple.argv[i].text);
        free(node->as.simple.argv);
        free_redirs(node->as.simple.redirs);
    }
    else if (node->type == AST_PIPELINE)
    {
        for (i = 0; i < node->as.pipeline.count; i++)
            ast_free(node->as.pipeline.items[i]);
        free(node->as.pipeline.items);
    }
    else
    {
        ast_free(node->as.binary.left);
        ast_free(node->as.binary.right);
    }
    free(node);
}
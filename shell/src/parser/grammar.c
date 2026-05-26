#include <stdlib.h>
#include "parser/grammar.h"
#include "parser/parse_error.h"

static t_token *cur(t_parser *p)
{
    return &p->ts->items[p->ts->cursor];
}

static int accept(t_parser *p, t_token_type type)
{
    if (cur(p)->type == type)
    {
        p->ts->cursor++;
        return 1;
    }
    return 0;
}

static int redir_type_from_token(t_token_type t, t_redir_type *out, int *fd)
{
    if (t == TOK_REDIR_IN) { *out = R_IN; *fd = 0; return 1; }
    if (t == TOK_REDIR_OUT) { *out = R_OUT; *fd = 1; return 1; }
    if (t == TOK_REDIR_APPEND) { *out = R_APPEND; *fd = 1; return 1; }
    if (t == TOK_REDIR_ERR) { *out = R_ERR_OUT; *fd = 2; return 1; }
    if (t == TOK_REDIR_ERR_APP) { *out = R_ERR_APPEND; *fd = 2; return 1; }
    return 0;
}

static int parse_redirection(t_parser *p, t_ast *node)
{
    t_redir_type rt;
    int fd;

    if (!redir_type_from_token(cur(p)->type, &rt, &fd))
        return 0;
    p->ts->cursor++;
    if (cur(p)->type != TOK_WORD)
    {
        parse_error_unexpected(cur(p)->lexeme);
        return 1;
    }
    if (ast_simple_add_redir(node, rt, fd, cur(p)->lexeme, cur(p)->quoted))
        return 1;
    p->ts->cursor++;
    return 0;
}

static int parse_list_operator(t_parser *p, t_ast_type *bin_type)
{
    if (accept(p, TOK_AND_IF))
        *bin_type = AST_AND;
    else if (accept(p, TOK_OR_IF))
        *bin_type = AST_OR;
    else if (accept(p, TOK_SEMI))
        *bin_type = AST_LIST;
    else
        return 0;
    return 1;
}

int parse_simple_command(t_parser *p, t_ast **out)
{
    t_ast *node = ast_new_simple();
    int saw_token = 0;

    if (!node)
        return 1;
    while (cur(p)->type == TOK_WORD || redir_type_from_token(cur(p)->type, &(t_redir_type){0}, &(int){0}))
    {
        saw_token = 1;
        if (cur(p)->type == TOK_WORD)
        {
            if (ast_simple_add_arg(node, cur(p)->lexeme, cur(p)->quoted))
                return ast_free(node), 1;
            p->ts->cursor++;
        }
        else
        {
            if (parse_redirection(p, node))
            {
                ast_free(node);
                return 1;
            }
        }
    }
    if (!saw_token)
    {
        parse_error_unexpected(cur(p)->lexeme);
        ast_free(node);
        return 1;
    }
    *out = node;
    return 0;
}

static int parse_list_tail(t_parser *p, t_ast **node)
{
    t_ast *rhs;
    t_ast_type bin_type;

    while (parse_list_operator(p, &bin_type))
    {
        if (parse_pipeline(p, &rhs))
            return 1;
        *node = ast_new_binary(bin_type, *node, rhs);
        if (!*node)
            return 1;
        if (accept(p, TOK_AMP))
            (*node)->type = AST_BACKGROUND;
    }
    return 0;
}

int parse_pipeline(t_parser *p, t_ast **out)
{
    t_ast *left;
    t_ast *pipe_node = NULL;

    if (parse_simple_command(p, &left))
        return 1;
    if (cur(p)->type != TOK_PIPE)
    {
        *out = left;
        return 0;
    }
    pipe_node = ast_new_pipeline();
    if (!pipe_node)
        return ast_free(left), 1;
    if (ast_pipeline_add(pipe_node, left))
        return ast_free(left), ast_free(pipe_node), 1;
    while (accept(p, TOK_PIPE))
    {
        t_ast *right;
        if (parse_simple_command(p, &right))
            return ast_free(pipe_node), 1;
        if (ast_pipeline_add(pipe_node, right))
            return ast_free(right), ast_free(pipe_node), 1;
    }
    *out = pipe_node;
    return 0;
}

int parse_list(t_parser *p, t_ast **out)
{
    t_ast *node;

    if (parse_pipeline(p, &node))
        return 1;
    if (accept(p, TOK_AMP))
        node->type = AST_BACKGROUND;
    if (parse_list_tail(p, &node))
        return ast_free(node), 1;
    if (cur(p)->type != TOK_EOF)
    {
        parse_error_unexpected(cur(p)->lexeme);
        ast_free(node);
        return 1;
    }
    *out = node;
    return 0;
}
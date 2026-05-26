#ifndef AST_AST_H
#define AST_AST_H

#include <stddef.h>
#include <stdbool.h>
#include "ast/ast_node.h"

typedef enum e_redir_type {
    R_IN,
    R_OUT,
    R_APPEND,
    R_ERR_OUT,
    R_ERR_APPEND
} t_redir_type;

typedef struct s_word {
    char *text;
    unsigned char quoted;
} t_word;

typedef struct s_redir {
    t_redir_type type;
    int          fd;
    t_word       target;
    struct s_redir *next;
} t_redir;

typedef struct s_simple_cmd {
    t_word  *argv;
    size_t   argc;
    t_redir *redirs;
    bool     background;
} t_simple_cmd;

typedef struct s_ast t_ast;

struct s_ast {
    t_ast_type type;
    union {
        t_simple_cmd simple;
        struct {
            t_ast  **items;
            size_t   count;
        } pipeline;
        struct {
            t_ast *left;
            t_ast *right;
        } binary;
    } as;
};

t_ast *ast_new_simple(void);
t_ast *ast_new_pipeline(void);
t_ast *ast_new_binary(t_ast_type type, t_ast *left, t_ast *right);
int    ast_simple_add_arg(t_ast *node, const char *text, unsigned char quoted);
int    ast_simple_add_redir(t_ast *node, t_redir_type type, int fd, const char *target, unsigned char quoted);
int    ast_pipeline_add(t_ast *node, t_ast *child);
char **ast_simple_argv_dup(const t_simple_cmd *cmd);
void   ast_free(t_ast *node);

#endif
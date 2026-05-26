#ifndef AST_AST_NODE_H
#define AST_AST_NODE_H

typedef enum e_ast_type {
    AST_SIMPLE,
    AST_PIPELINE,
    AST_LIST,
    AST_AND,
    AST_OR,
    AST_BACKGROUND
} t_ast_type;

#endif
#include <stdio.h>
#include "ast/ast_print.h"

static void indent(int depth)
{
    while (depth-- > 0)
        printf("  ");
}

void ast_print(const t_ast *node, int depth)
{
    size_t i;

    if (!node)
        return;
    indent(depth);
    printf("type=%d\n", node->type);
    if (node->type == AST_SIMPLE || node->type == AST_BACKGROUND)
    {
        for (i = 0; i < node->as.simple.argc; i++)
        {
            indent(depth + 1);
            printf("argv[%zu]=%s\n", i, node->as.simple.argv[i].text);
        }
    }
    else if (node->type == AST_PIPELINE)
    {
        for (i = 0; i < node->as.pipeline.count; i++)
            ast_print(node->as.pipeline.items[i], depth + 1);
    }
    else
    {
        ast_print(node->as.binary.left, depth + 1);
        ast_print(node->as.binary.right, depth + 1);
    }
}
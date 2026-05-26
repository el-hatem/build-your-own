#include <stdlib.h>
#include "shell/shell.h"
#include "input/input.h"
#include "input/history.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "expand/expand.h"
#include "exec/exec.h"

int shell_run(t_shell *sh)
{
    char *line;
    t_token_stream ts;
    t_parser parser;
    t_ast *ast = NULL;
    t_exec_ctx ctx;

    while (!sh->should_exit)
    {
        jobs_reap_completed(sh);
        line = input_read_line(sh, "$ ");
        if (!line)
            break;
        if (*line == '\0')
        {
            free(line);
            continue;
        }
        history_add_entry(sh, line);
        if (lexer_tokenize(line, &ts) == 0 && parser_init(&parser, &ts) == 0
            && parser_parse(&parser, &ast) == 0)
        {
            expand_ast(sh, ast);
            ctx.interactive = sh->interactive;
            ctx.in_pipeline = false;
            ctx.background = false;
            sh->last_status = exec_ast(sh, ast, &ctx);
            ast_free(ast);
            token_stream_free(&ts);
        }
        free(line);
    }
    history_save(sh, 1);
    return 0;
}
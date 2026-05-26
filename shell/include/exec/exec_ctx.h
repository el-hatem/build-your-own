#ifndef EXEC_EXEC_CTX_H
#define EXEC_EXEC_CTX_H

#include <stdbool.h>

typedef struct s_exec_ctx {
    bool interactive;
    bool in_pipeline;
    bool background;
} t_exec_ctx;

#endif
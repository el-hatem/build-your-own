#ifndef UTIL_ARENA_H
#define UTIL_ARENA_H

#include <stddef.h>

typedef struct s_arena_chunk {
    void *mem;
    struct s_arena_chunk *next;
} t_arena_chunk;

typedef struct s_arena {
    t_arena_chunk *chunks;
} t_arena;

int   arena_init(t_arena *arena);
void *arena_alloc(t_arena *arena, size_t size);
void  arena_reset(t_arena *arena);
void  arena_destroy(t_arena *arena);

#endif
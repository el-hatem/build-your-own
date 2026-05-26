#include <stdlib.h>

#include "util/arena.h"

int arena_init(t_arena *arena)
{
    arena->chunks = NULL;
    return 0;
}

void *arena_alloc(t_arena *arena, size_t size)
{
    t_arena_chunk *chunk = malloc(sizeof(*chunk));
    if (!chunk)
        return NULL;
    chunk->mem = malloc(size);
    if (!chunk->mem)
    {
        free(chunk);
        return NULL;
    }
    chunk->next = arena->chunks;
    arena->chunks = chunk;
    return chunk->mem;
}

void arena_reset(t_arena *arena)
{
    t_arena_chunk *cur = arena->chunks;
    t_arena_chunk *next;

    while (cur)
    {
        next = cur->next;
        free(cur->mem);
        free(cur);
        cur = next;
    }
    arena->chunks = NULL;
}

void arena_destroy(t_arena *arena)
{
    arena_reset(arena);
}
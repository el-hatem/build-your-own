#include <stdlib.h>

#include "util/strvec.h"

int strvec_init(t_strvec *vec)
{
    vec->items = NULL;
    vec->len = 0;
    vec->cap = 0;
    return 0;
}

int strvec_push(t_strvec *vec, char *s)
{
    char **new_items;
    size_t new_cap;

    if (vec->len + 1 >= vec->cap)
    {
        new_cap = vec->cap ? vec->cap * 2 : 8;
        new_items = realloc(vec->items, new_cap * sizeof(*new_items));
        if (!new_items)
            return 1;
        vec->items = new_items;
        vec->cap = new_cap;
    }
    vec->items[vec->len++] = s;
    vec->items[vec->len] = NULL;
    return 0;
}

void strvec_free(t_strvec *vec)
{
    size_t i = 0;
    while (i < vec->len)
    {
        free(vec->items[i]);
        i++;
    }
    free(vec->items);
    vec->items = NULL;
    vec->len = 0;
    vec->cap = 0;
}
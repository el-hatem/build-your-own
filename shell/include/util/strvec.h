#ifndef UTIL_STRVEC_H
#define UTIL_STRVEC_H

#include <stddef.h>

typedef struct s_strvec {
    char   **items;
    size_t   len;
    size_t   cap;
} t_strvec;

int  strvec_init(t_strvec *vec);
int  strvec_push(t_strvec *vec, char *s);
void strvec_free(t_strvec *vec);

#endif
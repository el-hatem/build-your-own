#ifndef BUILTINS_BUILTIN_REGISTRY_H
#define BUILTINS_BUILTIN_REGISTRY_H

#include <stddef.h>
#include <stdbool.h>

struct s_shell;
typedef int (*t_builtin_fn)(struct s_shell *sh, char **argv);

typedef struct s_builtin_spec {
    const char   *name;
    t_builtin_fn  fn;
    bool          parent_only;
} t_builtin_spec;

typedef struct s_builtin_registry {
    t_builtin_spec *items;
    size_t          count;
} t_builtin_registry;

int builtin_register_defaults(struct s_shell *sh);
const t_builtin_spec *builtin_find(struct s_shell *sh, const char *name);

#endif
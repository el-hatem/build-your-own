#ifndef BUILTINS_BUILTINS_H
#define BUILTINS_BUILTINS_H

struct s_shell;
int builtin_dispatch(struct s_shell *sh, char **argv);
int builtin_is_parent_only(const char *name);

#endif
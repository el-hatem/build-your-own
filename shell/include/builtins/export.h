#ifndef BUILTINS_EXPORT_H
#define BUILTINS_EXPORT_H

struct s_shell;
int bi_export(struct s_shell *sh, char **argv);

#endif
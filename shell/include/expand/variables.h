#ifndef EXPAND_VARIABLES_H
#define EXPAND_VARIABLES_H

struct s_shell;
char *expand_variables(struct s_shell *sh, const char *input, unsigned char quoted);
int   shell_set_local(struct s_shell *sh, const char *name, const char *value);
int   shell_set_exported(struct s_shell *sh, const char *name, const char *value);
const char *shell_get_var(struct s_shell *sh, const char *name);
int   shell_init_env(struct s_shell *sh, char **envp);
void  shell_destroy_env(struct s_shell *sh);
char **shell_build_envp(struct s_shell *sh);

#endif
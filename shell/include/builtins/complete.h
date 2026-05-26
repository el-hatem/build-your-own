#ifndef BUILTINS_COMPLETE_H
#define BUILTINS_COMPLETE_H

struct s_shell;
int bi_complete(struct s_shell *sh, char **argv);
void bi_complete_destroy(void);
const char *complete_get_script(const char *command);
int complete_unregister(const char *command);

#endif
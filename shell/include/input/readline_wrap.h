#ifndef INPUT_READLINE_WRAP_H
#define INPUT_READLINE_WRAP_H

struct s_shell;
int   readline_wrap_init(struct s_shell *sh);
char *readline_wrap_read(const char *prompt);

#endif
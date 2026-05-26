#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

struct s_shell;
char *input_read_line(struct s_shell *sh, const char *prompt);

#endif
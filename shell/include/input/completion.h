#ifndef INPUT_COMPLETION_H
#define INPUT_COMPLETION_H

struct s_shell;
int   completion_init(struct s_shell *sh);
char **shell_completion(const char *text, int start, int end);
char  *command_generator(const char *text, int state);
char  *programmable_completion_generator(const char *text, int state);

#endif
#ifndef INPUT_HISTORY_H
#define INPUT_HISTORY_H

#include <stddef.h>

typedef struct s_history {
    char   *path;
    size_t  loaded;
} t_history;

struct s_shell;
int history_init(struct s_shell *sh, const char *path);
int history_load(struct s_shell *sh);
int history_save(struct s_shell *sh, int append_mode);
int history_add_entry(struct s_shell *sh, const char *line);
int history_command(struct s_shell *sh, char **argv);
void history_destroy(struct s_shell *sh);

#endif
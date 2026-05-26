#ifndef EXEC_SEARCH_PATH_H
#define EXEC_SEARCH_PATH_H

struct s_shell;
char *search_in_path(struct s_shell *sh, const char *name);

#endif
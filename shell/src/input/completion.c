#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <readline/readline.h>

#include "input/completion.h"
#include "shell/shell.h"
#include "builtins/builtin_registry.h"
#include "builtins/complete.h"
#include "expand/variables.h"

typedef struct s_match_list {
    struct s_match_entry {
        char *text;
        char append_character;
    } *items;
    size_t count;
} t_match_list;

static struct s_shell *g_shell;
static char g_path_dir[1024];
static char g_path_prefix[1024];
static char g_path_replace[1024];

static char *dup_string(const char *s)
{
    char *copy;

    copy = malloc(strlen(s) + 1);
    if (!copy)
        return NULL;
    strcpy(copy, s);
    return copy;
}

static int is_directory_path(const char *path)
{
    struct stat st;

    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int match_list_has(const t_match_list *matches, const char *name);

static int is_word_separator(char c)
{
    return c == ' ' || c == '\t';
}

static void completion_get_first_word(char *dst, size_t dst_size)
{
    size_t i;
    size_t len;

    if (dst_size == 0)
        return;
    i = 0;
    while (rl_line_buffer[i] && is_word_separator(rl_line_buffer[i]))
        i++;
    len = 0;
    while (rl_line_buffer[i + len] && !is_word_separator(rl_line_buffer[i + len]))
        len++;
    if (len >= dst_size)
        len = dst_size - 1;
    memcpy(dst, rl_line_buffer + i, len);
    dst[len] = '\0';
}

static void free_match_list(t_match_list *matches)
{
    size_t i = 0;

    while (i < matches->count)
    {
        free(matches->items[i].text);
        i++;
    }
    free(matches->items);
    matches->items = NULL;
    matches->count = 0;
}

static int match_list_add(t_match_list *matches, const char *name, char append_character)
{
    struct s_match_entry *new_items;
    char *copy;

    if (match_list_has(matches, name))
        return 0;
    copy = dup_string(name);
    if (!copy)
        return 1;
    new_items = realloc(matches->items, (matches->count + 1) * sizeof(*new_items));
    if (!new_items)
    {
        free(copy);
        return 1;
    }
    matches->items = new_items;
    matches->items[matches->count].text = copy;
    matches->items[matches->count].append_character = append_character;
    matches->count++;
    return 0;
}

static int match_list_compare(const void *lhs, const void *rhs)
{
    const struct s_match_entry *left;
    const struct s_match_entry *right;

    left = lhs;
    right = rhs;
    return strcmp(left->text, right->text);
}

static void match_list_sort(t_match_list *matches)
{
    if (matches->count > 1)
        qsort(matches->items, matches->count, sizeof(*matches->items), match_list_compare);
}

static int match_list_has(const t_match_list *matches, const char *name)
{
    size_t i = 0;

    while (i < matches->count)
    {
        if (strcmp(matches->items[i].text, name) == 0)
            return 1;
        i++;
    }
    return 0;
}

static void collect_builtin_matches(const char *text, t_match_list *matches)
{
    size_t i = 0;
    size_t len = strlen(text);

    while (i < g_shell->builtins.count)
    {
        if (strncmp(g_shell->builtins.items[i].name, text, len) == 0)
            match_list_add(matches, g_shell->builtins.items[i].name, ' ');
        i++;
    }
}

static void collect_path_matches_from_dir(const char *dir_path, const char *text, t_match_list *matches)
{
    DIR *dir;
    struct dirent *de;
    char path[4096];
    size_t len = strlen(text);

    dir = opendir(dir_path);
    if (!dir)
        return;
    while ((de = readdir(dir)) != NULL)
    {
        if (de->d_name[0] == '.' && text[0] != '.')
            continue;
        if (strncmp(de->d_name, text, len) != 0)
            continue;
        if (match_list_has(matches, de->d_name))
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir_path, de->d_name);
        if (access(path, X_OK) == 0)
            match_list_add(matches, de->d_name, ' ');
    }
    closedir(dir);
}

static void collect_path_matches(const char *text, t_match_list *matches)
{
    const char *path_env;
    char *paths;
    char *save;
    char *dir_path;

    path_env = shell_get_var(g_shell, "PATH");
    if (!path_env)
        return;
    paths = dup_string(path_env);
    if (!paths)
        return;
    save = NULL;
    dir_path = strtok_r(paths, ":", &save);
    while (dir_path)
    {
        collect_path_matches_from_dir(dir_path, text, matches);
        dir_path = strtok_r(NULL, ":", &save);
    }
    free(paths);
}

static char *command_generator_impl(const char *text, int state)
{
    static t_match_list matches;
    static size_t index;

    if (state == 0)
    {
        free_match_list(&matches);
        collect_builtin_matches(text, &matches);
        collect_path_matches(text, &matches);
        match_list_sort(&matches);
        index = 0;
    }
    if (index < matches.count)
    {
        rl_completion_append_character = matches.items[index].append_character;
        return dup_string(matches.items[index++].text);
    }
    return NULL;
}

static void completion_find_token_bounds(int *start_out, int *last_slash_out)
{
    const char *line;
    int point;
    int start;
    int last_slash;
    int i;

    line = rl_line_buffer;
    point = rl_point;
    start = point;
    last_slash = -1;
    while (start > 0 && line[start - 1] != ' ' && line[start - 1] != '\t')
        start--;
    i = start;
    while (i < point)
    {
        if (line[i] == '/')
            last_slash = i;
        i++;
    }
    *start_out = start;
    *last_slash_out = last_slash;
}

static void completion_copy_range(char *dst, size_t dst_size, const char *src, size_t len)
{
    if (len >= dst_size)
        len = dst_size - 1;
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void completion_set_path_parts(void)
{
    int start;
    int last_slash;
    size_t dir_len;
    size_t replace_len;

    completion_find_token_bounds(&start, &last_slash);
    if (last_slash < 0)
    {
        strcpy(g_path_dir, ".");
        completion_copy_range(g_path_prefix, sizeof(g_path_prefix), rl_line_buffer + start, (size_t)rl_point - (size_t)start);
        g_path_replace[0] = '\0';
        return;
    }
    dir_len = (size_t)last_slash - (size_t)start;
    if (dir_len == 0)
        strcpy(g_path_dir, "/");
    else
        completion_copy_range(g_path_dir, sizeof(g_path_dir), rl_line_buffer + start, dir_len);
    replace_len = (size_t)last_slash - (size_t)start + 1;
    completion_copy_range(g_path_replace, sizeof(g_path_replace), rl_line_buffer + start, replace_len);
    completion_copy_range(g_path_prefix, sizeof(g_path_prefix), rl_line_buffer + last_slash + 1, strlen(rl_line_buffer + last_slash + 1));
}

static int path_match_add_entry(t_match_list *matches, struct dirent *de)
{
    char path[4096];
    int is_dir;
    char candidate[4096];

    if (de->d_name[0] == '.' && g_path_prefix[0] != '.')
        return 0;
    snprintf(path, sizeof(path), "%s/%s", g_path_dir, de->d_name);
    is_dir = is_directory_path(path);
    if (strncmp(de->d_name, g_path_prefix, strlen(g_path_prefix)) != 0)
        return 0;
    if (snprintf(candidate, sizeof(candidate), "%s%s", g_path_replace, de->d_name) >= (int)sizeof(candidate))
        return 1;
    if (is_dir)
    {
        size_t len;

        len = strlen(candidate);
        if (len + 1 >= sizeof(candidate))
            return 1;
        candidate[len] = '/';
        candidate[len + 1] = '\0';
    }
    return match_list_add(matches, candidate, is_dir ? '\0' : ' ');
}

static char *path_generator(const char *text, int state)
{
    static DIR *dir;
    static t_match_list matches;
    static size_t index;
    char *match;
    struct dirent *de;

    (void)text;
    if (state == 0)
    {
        if (dir)
            closedir(dir);
        free_match_list(&matches);
        completion_set_path_parts();
        dir = opendir(g_path_dir);
        if (!dir)
            return NULL;
        while ((de = readdir(dir)) != NULL)
            path_match_add_entry(&matches, de);
        match_list_sort(&matches);
        index = 0;
        closedir(dir);
        dir = NULL;
    }
    if (index < matches.count)
    {
        rl_completion_append_character = matches.items[index].append_character;
        return dup_string(matches.items[index++].text);
    }
    return NULL;
}

int completion_init(struct s_shell *sh)
{
    g_shell = sh;
    rl_attempted_completion_function = shell_completion;
    return 0;
}

char **shell_completion(const char *text, int start, int end)
{
    char first_word[1024];

    (void)end;
    if (start == 0 && strpbrk(rl_line_buffer, " \t") == NULL)
    {
        rl_completion_append_character = ' ';
        return rl_completion_matches(text, command_generator_impl);
    }
    completion_get_first_word(first_word, sizeof(first_word));
    if (start > 0 && first_word[0] != '\0')
    {
        const char *script;

        script = complete_get_script(first_word);
        if (script)
        {
            rl_completion_append_character = ' ';
            return rl_completion_matches(text, programmable_completion_generator);
        }
    }
    rl_completion_append_character = ' ';
    return rl_completion_matches(text, path_generator);
}

char *command_generator(const char *text, int state)
{
    return command_generator_impl(text, state);
}
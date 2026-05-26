#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <readline/readline.h>

#include "builtins/complete.h"

typedef struct s_word_list {
    char **items;
    size_t count;
} t_word_list;

typedef struct s_candidate_list {
    char **items;
    size_t count;
} t_candidate_list;

static char *dup_string(const char *value)
{
    char *copy;

    copy = malloc(strlen(value) + 1);
    if (!copy)
        return NULL;
    strcpy(copy, value);
    return copy;
}

static int is_word_separator(char c)
{
    return c == ' ' || c == '\t';
}

static void free_word_list(t_word_list *words)
{
    size_t i;

    i = 0;
    while (i < words->count)
    {
        free(words->items[i]);
        i++;
    }
    free(words->items);
    words->items = NULL;
    words->count = 0;
}

static int word_list_add(t_word_list *words, const char *start, size_t len)
{
    char **new_items;
    char *copy;

    copy = malloc(len + 1);
    if (!copy)
        return 1;
    memcpy(copy, start, len);
    copy[len] = '\0';
    new_items = realloc(words->items, (words->count + 1) * sizeof(*new_items));
    if (!new_items)
    {
        free(copy);
        return 1;
    }
    words->items = new_items;
    words->items[words->count++] = copy;
    return 0;
}

static int collect_words(t_word_list *words)
{
    size_t i;
    size_t start;

    i = 0;
    while (i < (size_t)rl_point)
    {
        while (i < (size_t)rl_point && is_word_separator(rl_line_buffer[i]))
            i++;
        start = i;
        while (i < (size_t)rl_point && !is_word_separator(rl_line_buffer[i]))
            i++;
        if (i > start && word_list_add(words, rl_line_buffer + start, i - start))
            return 1;
    }
    return 0;
}

static int candidates_add(t_candidate_list *matches, const char *value)
{
    char **new_items;
    char *copy;

    copy = dup_string(value);
    if (!copy)
        return 1;
    new_items = realloc(matches->items, (matches->count + 1) * sizeof(*new_items));
    if (!new_items)
    {
        free(copy);
        return 1;
    }
    matches->items = new_items;
    matches->items[matches->count++] = copy;
    return 0;
}

static const char *word_or_empty(const t_word_list *words, size_t index)
{
    if (index >= words->count)
        return "";
    return words->items[index];
}

static void free_candidate_list(t_candidate_list *matches)
{
    size_t i;

    i = 0;
    while (i < matches->count)
    {
        free(matches->items[i]);
        i++;
    }
    free(matches->items);
    matches->items = NULL;
    matches->count = 0;
}

static int read_candidates_from_pipe(int fd, t_candidate_list *matches);

static size_t completion_env_cword(size_t word_count)
{
    if (word_count == 0)
        return 0;
    return word_count - 1;
}

static int set_env_value(const char *name, const char *value)
{
    return setenv(name, value, 1) != 0;
}

static int set_completion_env(const char *line, size_t point, size_t word_count)
{
    char point_buf[32];
    char cword_buf[32];
    int point_len;
    int cword_len;
    const char *names[] = {"COMP_LINE", "COMP_POINT", "COMP_KEY", "COMP_TYPE", "COMP_CWORD"};
    const char *values[5];
    size_t i;

    point_len = snprintf(point_buf, sizeof(point_buf), "%zu", point);
    cword_len = snprintf(cword_buf, sizeof(cword_buf), "%zu", completion_env_cword(word_count));
    if (point_len < 0 || cword_len < 0)
        return 1;
    values[0] = line;
    values[1] = point_buf;
    values[2] = "9";
    values[3] = "63";
    values[4] = cword_buf;
    i = 0;
    while (i < sizeof(names) / sizeof(names[0]))
    {
        if (set_env_value(names[i], values[i]))
            return 1;
        i++;
    }
    return 0;
}

static char **build_exec_argv(const char *script, const char *command, const char *current, const char *previous)
{
    char **argv;

    argv = calloc(5, sizeof(*argv));
    if (!argv)
        return NULL;
    argv[0] = (char *)script;
    argv[1] = (char *)command;
    argv[2] = (char *)current;
    argv[3] = (char *)previous;
    return argv;
}

static const char *completion_previous_word(const t_word_list *words, const char *current_word)
{
    if (!current_word || current_word[0] == '\0')
        return word_or_empty(words, words->count > 0 ? words->count - 1 : 0);
    return word_or_empty(words, words->count > 1 ? words->count - 2 : 0);
}

static int start_completion_script(const char *script, const char *command, const char *current_word, const t_word_list *words, int pipefd[2], char ***argv_out)
{
    const char *previous_word;

    previous_word = completion_previous_word(words, current_word);
    *argv_out = build_exec_argv(script, command, current_word ? current_word : "", previous_word);
    if (!*argv_out)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        return 1;
    }
    return 0;
}

static int prepare_completion_script(const char *script, const char *current_word, t_word_list *words, int pipefd[2], char ***argv_out)
{
    if (collect_words(words))
        return 1;
    if (set_completion_env(rl_line_buffer, (size_t)rl_point, words->count))
        return 1;
    if (pipe(pipefd) != 0)
        return 1;
    if (start_completion_script(script, word_or_empty(words, 0), current_word, words, pipefd, argv_out))
        return 1;
    return 0;
}

static int run_completion_child(const char *script, int pipe_write, char **argv)
{
    if (dup2(pipe_write, STDOUT_FILENO) < 0)
        _exit(127);
    close(pipe_write);
    execvp(script, argv);
    _exit(127);
}

static int finish_completion_script(pid_t pid, int pipe_read, char **argv, t_candidate_list *matches)
{
    int status;
    int rc;

    rc = read_candidates_from_pipe(pipe_read, matches);
    close(pipe_read);
    free(argv);
    if (waitpid(pid, &status, 0) < 0)
        return 1;
    return rc;
}

static int append_stdout_line(t_candidate_list *matches, const char *line)
{
    size_t len;

    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        len--;
    if (len == 0)
        return 0;
    {
        char *copy;

        copy = malloc(len + 1);
        if (!copy)
            return 1;
        memcpy(copy, line, len);
        copy[len] = '\0';
        if (candidates_add(matches, copy))
        {
            free(copy);
            return 1;
        }
        free(copy);
    }
    return 0;
}

static int read_candidates_from_pipe(int fd, t_candidate_list *matches)
{
    FILE *stream;
    char *line;
    size_t capacity;
    ssize_t length;
    int rc;

    stream = fdopen(fd, "r");
    if (!stream)
        return 1;
    line = NULL;
    capacity = 0;
    rc = 0;
    while ((length = getline(&line, &capacity, stream)) != -1)
    {
        line[length] = '\0';
        if (append_stdout_line(matches, line))
        {
            rc = 1;
            break;
        }
    }
    free(line);
    fclose(stream);
    return rc;
}

static int run_completion_script(const char *script, const char *current_word, t_candidate_list *matches)
{
    t_word_list words;
    char **argv;
    int pipefd[2];
    pid_t pid;

    words.items = NULL;
    words.count = 0;
    if (prepare_completion_script(script, current_word, &words, pipefd, &argv))
    {
        free_word_list(&words);
        return 1;
    }
    pid = fork();
    if (pid == 0)
    {
        close(pipefd[0]);
        run_completion_child(script, pipefd[1], argv);
    }
    free_word_list(&words);
    close(pipefd[1]);
    if (pid < 0)
    {
        close(pipefd[0]);
        free(argv);
        return 1;
    }
    return finish_completion_script(pid, pipefd[0], argv, matches);
}

static const char *programmable_completion_script(const char *text)
{
    char first_word[1024];
    size_t i;
    size_t len;

    i = 0;
    while (rl_line_buffer[i] == ' ' || rl_line_buffer[i] == '\t')
        i++;
    len = 0;
    while (rl_line_buffer[i + len] && rl_line_buffer[i + len] != ' ' && rl_line_buffer[i + len] != '\t')
        len++;
    if (len >= sizeof(first_word))
        len = sizeof(first_word) - 1;
    memcpy(first_word, rl_line_buffer + i, len);
    first_word[len] = '\0';
    if (first_word[0] == '\0')
        return NULL;
    (void)text;
    return complete_get_script(first_word);
}

static int programmable_completion_prepare(const char *text, t_candidate_list *matches, const char **script_out)
{
    *script_out = programmable_completion_script(text);
    if (!*script_out)
        return 1;
    if (run_completion_script(*script_out, text, matches))
        return 1;
    return 0;
}

char *programmable_completion_generator(const char *text, int state)
{
    static t_candidate_list matches;
    static size_t index;
    static const char *script;

    if (state == 0)
    {
        free_candidate_list(&matches);
        if (programmable_completion_prepare(text, &matches, &script))
        {
            free_candidate_list(&matches);
            index = 0;
            return NULL;
        }
        index = 0;
    }
    if (index < matches.count)
    {
        rl_completion_append_character = ' ';
        return dup_string(matches.items[index++]);
    }
    return NULL;
}
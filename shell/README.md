# shell

A modular Unix-like shell implemented in C with a REPL loop, tokenization, parsing, expansion, built-ins, pipelines, redirection, background jobs, and Readline-based interactive input support. The design follows the standard shell processing model of reading input, tokenizing, parsing, expanding, redirecting, executing, and collecting status, which is the right foundation for adding advanced shell behavior incrementally. [1][2][3]

## Features

- Interactive REPL using GNU Readline for line editing and command history. [3][4]
- Lexer and parser split into separate modules for cleaner architecture and future extensibility. [2][1]
- Built-in commands such as `cd`, `exit`, `pwd`, `echo`, `export`, `declare`, `jobs`, `history`, and `complete`, with parent-only builtins executed in the shell process so they can mutate shell state correctly. [1][2]
- External command execution using the usual `fork`/`exec`/`waitpid` model. [1]
- Support for redirection, pipelines, background execution, simple variable expansion, and persistent history. [1][3]

## Project layout

```text
shell/
├── include/
│   ├── ast/
│   ├── builtins/
│   ├── exec/
│   ├── expand/
│   ├── input/
│   ├── jobs/
│   ├── lexer/
│   ├── parser/
│   ├── shell/
│   └── util/
└── src/
    ├── ast/
    ├── builtins/
    ├── exec/
    ├── expand/
    ├── input/
    ├── jobs/
    ├── lexer/
    ├── parser/
    ├── shell/
    └── util/
```

This separation keeps parsing logic isolated from execution logic and matches common shell architecture guidance, where lexer, parser, AST, execution, IO, and utility layers are intentionally decoupled. [2]

## Build requirements

### Dependencies

- C compiler with C11 support.
- GNU Readline development package, because linking with `-lreadline` alone is not enough; the project also needs the Readline header files at compile time. [5][6]
- A POSIX-like environment with `fork`, `execve`, `waitpid`, `pipe`, `dup2`, and `getline`. `getline` is standardized by POSIX.1-2008. [7]

### Install Readline headers

#### Debian / Ubuntu

```bash
sudo apt update
sudo apt install libreadline-dev
```

#### Fedora / RHEL / CentOS

```bash
sudo dnf install readline-devel
```

#### Arch Linux

```bash
sudo pacman -S readline
```

The exact package name varies by distribution, but the key requirement is the development package that provides `readline/readline.h` and `readline/history.h`. [5][8][9]

## Build

Compile the shell to a standalone binary such as `shell`:

```bash
gcc -g -D_POSIX_C_SOURCE=200809L -Iinclude $(find src -name '*.c') -o shell -lreadline
```

If the system requires an additional terminal library for Readline, use:

```bash
gcc -g -D_POSIX_C_SOURCE=200809L -Iinclude $(find src -name '*.c') -o shell -lreadline -lncurses
```

Readline-based applications are commonly linked this way, and `_POSIX_C_SOURCE=200809L` is appropriate when relying on modern POSIX interfaces such as `getline`. [3][7][10]

## Common build issues

### 1. `fatal error: readline/readline.h: No such file or directory`

This means the Readline development headers are missing or not in the compiler's include path. Installing the proper development package usually resolves it. [5][8]

### 2. `fatal error: readline/history.h: No such file or directory`

This is the same class of issue as above: the Readline history header is part of the development package, not the runtime linker flag. [8][9]

### 3. `invalid use of undefined type 'struct s_shell'`

This happens when a source file dereferences `sh->...` without including the full `shell/shell.h` definition. A forward declaration only tells the compiler that the struct exists; it does not expose the fields. [11]

Files that access shell state directly should include:

```c
#include "shell/shell.h"
```

In this project, that is especially important for modules such as:

- `src/jobs/jobs.c`
- `src/builtins/builtins.c`

## Running

After compiling:

```bash
./shell
```

Example session:

```bash
$ pwd
$ echo hello
$ ls | wc -l
$ export NAME=telast
$ echo $NAME
$ sleep 5 &
$ jobs
```

The shell reads a line, tokenizes operators and words, parses the command structure, expands variables, applies redirections, executes builtins or external programs, and then updates the final status. That execution order follows standard shell command language behavior. [1]

## Architecture overview

### Input layer

The input layer is responsible for interactive command reading and history integration through GNU Readline. Readline exists specifically to provide line editing, history, and completion hooks that standard C input does not provide. [3][4]

### Lexer

The lexer converts the raw command line into tokens such as words, pipes, logical operators, background markers, and redirections. Correct shell behavior depends on distinguishing operators from words before parsing begins. [1]

### Parser and AST

The parser turns tokens into an AST for simple commands, pipelines, command lists, logical operators, and background execution. Building an AST instead of executing directly from tokens makes extensions like conditionals, grouping, and more advanced expansion easier to add later. [2][1]

### Expansion

Expansion resolves shell variables such as `$VAR` and `${VAR}` before execution. In standard shell processing, expansion occurs after parsing and before redirection/execution. [1]

### Execution

The executor handles builtins, external commands, pipes, and redirections. Builtins that change shell state, such as `cd` or `export`, must run in the shell process itself, while external commands are typically run through `fork()` and `exec`. [1][2]

### Jobs and signals

Background jobs are tracked by the shell, and child cleanup is handled through `SIGCHLD` plus non-blocking `waitpid` reaping. That is the correct baseline for preventing zombie processes in a long-running shell. [1]

## Current builtins

| Builtin | Purpose |
|---|---|
| `cd` | Change current working directory. |
| `exit` | Exit the shell. |
| `pwd` | Print current directory. |
| `echo` | Print arguments. |
| `export` | Set exported environment variables. |
| `declare` | Set shell-local variables. |
| `jobs` | Show background jobs. |
| `history` | Show command history. |
| `complete` | Placeholder for programmable completion support. |

Builtins that modify shell state should stay parent-executed so their effects remain visible after the command returns. [1]

## Development notes

- Keep lexer, parser, expansion, and execution separate instead of collapsing them into one large `main()` or executor function. That separation is a core shell architecture recommendation because it makes behavior easier to reason about and extend. [2]
- Always check system call return values for `fork`, `exec`, `pipe`, `dup2`, `open`, and `waitpid`, because low-level shell code must assume every syscall can fail. [1]
- Free per-command allocations after each REPL cycle and close unused file descriptors immediately, especially around pipelines and redirections, to avoid leaks in a long-lived process. [1]
- Do not run the shell as root during development; shells directly execute external programs and manipulate process state, so safer privileges reduce the blast radius of bugs. [2]

## Roadmap

Planned or partial extensions include:

- More complete quoting and quote-removal rules.
- Stronger parameter expansion semantics.
- Rich programmable completion.
- More complete job control with process groups and stopped-job management.
- Improved history integration and persistence behavior.
- More POSIX-compatible parsing and expansion edge cases.

POSIX shell language behavior is substantially more complex than a minimal shell, especially around quoting, expansions, and grammar corner cases, so incremental refinement is the practical path. [1]


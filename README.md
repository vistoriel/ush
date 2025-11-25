# Ush

Ush is a small, educational implementation of a Unix-like interactive shell written in C. It implements a simple command loop, parsing and argument handling, a set of builtin commands, variable and tilde expansion, and basic job control for suspended processes.

## Features

- **Interactive prompt**: prints the prompt (variable `PROMPT`) and the current working directory in green.
- **Command parsing**: multiple commands separated by `;` are supported and trimmed.
- **Argument parsing**: respects quoted arguments (single, double and backtick quotes) and handles spaces inside quotes.
- **Variable expansion**: supports `${VAR}`, `$VAR` and `$?` (last status) expansions.
- **Tilde expansion**: `~` expands to the value of `HOME`.
- **Variable assignment and environment control**: bare assignments like `NAME=VALUE` are supported; use `export` and `unset` to promote and remove environment variables.
- **Built-in commands**: the shell provides these builtins (registered in `project/src/app.c`): `exit`, `bye`, `echo`, `export`, `unset`, `jobs`, `fg`, `pwd`, `cd`, `which`.
- **Echo flags**: `echo` supports `-n` (no trailing newline) and `-e` (interpret some backslash escapes like `\\n`, `\\t`, `\\\\`).
- **Job control (suspended processes)**: suspended processes (SIGTSTP) are tracked; use `jobs` to list and `fg` to resume a job.
- **External commands**: launches external programs with `execvp`, waits for them, and reports command-not-found errors.
- **Utilities**: `normalize_path` implementation handles `.` and `..` segments for `cd` and similar.

## Installation

### 1. Libmx library
Ensure the `libmx` implementation (sources and its Makefile) is available at `project/libmx`. The project Makefile will run `make -C libmx` to build `libmx.a`. If `project/libmx` is empty, copy or clone your `libmx` sources there.

### 2. Build the ush

Build the shell. You can build from the `project` directory or from the repository root.

From the `project` directory:
```
cd project
make
# the resulting binary will be `project/ush`
```

From repository root (top-level `Makefile`):
```
make
# the top-level `Makefile` will build the project and then offer to copy the executable to /usr/bin
```

### 3. Install the built executable to `/usr/bin` (optional)
To install the built executable to `/usr/bin` (top-level `make` already runs the `copy` target and will prompt for `sudo`):
```
sudo cp project/ush /usr/bin
```

### 4. Running

Run the shell directly from the `project` folder:
```
./ush
```

You can also pass command-line arguments to `ush`. The code can set a custom prompt from program arguments (see `project/src/main.c`).

## Examples / How to use

Run multiple commands on one line:
```
echo one; echo two
```

Simple external command:
```
ls
```

Use quoting and escaping:
```
echo "a string with spaces"
echo -e "line1\\nline2"
```

Variables and substitution:
```
FOO=hello
echo ${FOO}
echo $FOO
echo $?
```

Exporting and unsetting variables:
```
FOO=bar
export FOO
unset FOO
```

`cd` and `pwd` behaviors:
```
cd /tmp
pwd
cd -    # returns to previous working directory
```

`which` usage:
```
which ls
which -a ls   # show all matches in PATH
which -s ls   # uses physical resolution behavior
```

Job control:
```
sleep 100
# Press Ctrl-Z to suspend the job
jobs
fg       # bring the most recent suspended job to foreground
fg 1     # bring job number 1 to foreground
```

## Project layout (relevant files)

`project/src/` — C sources for the shell runtime (`main.c`, `app.c`, `command.c`, `exec.c`, `line.c`, `utils.c`, `var.c`, ...).

`project/inc/` — project headers (`ush.h`, `functions.h`, `structs.h`, `definitions.h`, ...).

`project/libmx/` — place your `libmx` source tree here before building; the project `Makefile` will build `libmx.a`.

`Makefile` — top-level convenience `Makefile` (builds and copies `project/ush` to `/usr/bin` with `sudo`).

`project/Makefile` — builds `libmx` and then compiles the shell object files and links with `libmx.a`.

## Running tests

There is a `tests` directory and helper scripts in the repo. To execute provided tests from the repository root, run:
```
make tests
```
Note: *ensure `libmx` sources are present and the project builds successfully before running tests.*

README

# Custom Shell Implementation

This project implements a custom shell in C, providing basic functionality similar to standard Unix shells. It supports both interactive and batch modes, along with features like command execution, built-in commands, and input/output redirection.

## Features

- Interactive and batch mode execution
- Built-in commands: `exit`, `cd`, and `path`
- External command execution
- Input/output redirection
- Parallel command execution

## Compilation

To compile the shell, use a C compiler like gcc:

```
gcc -o dash main.c
```

Replace `main.c` with the actual filename if different.

## Usage

### Interactive Mode

Run the shell without any arguments:

```
./dash
```

You will be presented with a prompt `dash>` where you can enter commands.

### Batch Mode

Run the shell with a batch file as an argument:

```
./dash batch_file.txt
```

The shell will execute commands from the specified file.

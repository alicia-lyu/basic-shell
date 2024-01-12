# Basic Shell

## Features

- Basic Shell Functionality
    - Interactive command line interface.
    - Executes commands and waits for completion.
    - Built-in commands: exit, cd, pwd.
- Advanced Features
    - Redirection: Supports standard output redirection using >.
    - Multiple Commands: Execute multiple commands in a single line, separated by ;.
    - Pipes: Allows output of one command as input to another using |.
    - Loops: Execute a command multiple times sequentially using loop command.

## Implementation Details

- Language: C.
- System Calls: Used fork, exec, wait, getline, strsep, chdir, getcwd.
- Error Handling: Uniform error message for all errors.
- Parsing: Separate process for parsing and execution to handle syntax errors.

## Usage

- To start the shell: ./smash
- Example commands:
    - `./smash`
    - `smash> /bin/ls -la /tmp > output`
    - `smash> loop 3 cmd args`

Developed and tested on Linux.

Acknowledgments: Thanks to CS 537 Spring 2023 course staff at University of Wisconsin–Madison for providing project description and test cases.
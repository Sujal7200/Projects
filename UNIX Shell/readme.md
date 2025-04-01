# Minimal Shell (`msh`)

## Overview
The Minimal Shell (`msh`) is a lightweight Unix shell implementation that provides fundamental command execution capabilities, including built-in commands, process creation, and I/O redirection. This project demonstrates key operating system concepts such as process control, file handling, and system calls.
The standalone file is not executable by itself, supporting files are stored in a shared repository. 

## Features
- **Command Execution**: Supports executing system commands via `execvp()`.
- **Built-in Commands**:
  - `exit`/`quit`: Terminates the shell.
  - `cd`: Changes the working directory.
- **Process Control**: Uses `fork()` and `waitpid()` to manage child processes.
- **I/O Redirection**: Implements output redirection using `dup2()` and file descriptors.
- **Batch Mode Execution**: Accepts a script file as input for automated command execution.

## Operating System Concepts Utilized
- **Process Management**: The shell uses `fork()` to create child processes and `execvp()` to replace the child process with the executed command.
- **Interprocess Communication (IPC)**: Parent processes wait for child processes to complete using `waitpid()`.
- **File System Navigation**: The `cd` command utilizes `chdir()` to modify the current working directory.
- **File Redirection**: Implements redirection using `dup2()` to manipulate file descriptors.
- **Error Handling**: Provides robust error messages for invalid commands and redirection misuse.

## Future Enhancements
- Implement command history and recall.
- Support for background process execution (`&` operator).
- Improved parsing with support for pipes (`|`).

## License
This project is licensed under the MIT License.

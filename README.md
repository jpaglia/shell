# Basic Shell in C

This project consists of designing a C program to serve as a simple shell interface that accepts user commands and then executes each command in a separate process. The implementation supports input and output redirection, as well as pipes as a form of IPC between a pair of commands. The project also involves the use of the UNIX `fork()`, `exec()`, `wait()`, `dup2()`, and `pipe()` system calls. This shell was developed and testing on an Ubuntu VM.

### Compiling and Running the Shell

Clone this project and navigate to its working directory. In your terminal, execute the following command:
> `make jpags`

### Overview

This shell supports each of the following features and functionalities: 
1. Creates the child process and executed the command in the child 
2. History feature 
3. Input and output redirection 
4. Allows the parent and child processes to communicate via a pipe
5. Allows background processes

### Executing Commands
Below is a list of commands that this shell supports:
1. Basic shell commands such as `ls`, `mkdir`, `cat`, etc.
2. `cd <DIR_NAME>`
3. Background processes via `&`
4. History feature via `!!`
5. Redirection via `<` or `>`
6. Piping via `|`

### Creating a History Feature
The implementation of this feature provides a **history** to allow a user to execute the most recent command by entering `!!`. For example, if a user enters the command `ls -l`, they can then execute that command again by entering `!!` in the prompt. Any command executed in this fashion is also *echoed* on the user’s screen, and the command is placed in the history buffer as the next command. This shell also manages basic error handling. If there is no recent command in the history, entering `!!` will result in a message 
“No commands in history.”

### Redirecting Input and Output
This shell also supports the `>` and `<` redirection operators, where `>` redirects the output of a command to a file and `<` redirects the input to a command from a file. For example, if a user enters ```mysh:~$ ls > out.txt``` the output from the ls command will be redirected to the file `out.txt`. Similarly, input can be redirected as well. For example, if the user enters ```mysh:~$ sort < in.txt``` the file `in.txt` will serve as input to the `sort` command. Managing the redirection of both input and output involves using the `dup2()` function, which duplicates an existing file descriptor to another file descriptor. For example, if fd is a file descriptor to the file `out.txt`, the call ```dup2(fd, STDOUT FILENO);``` duplicates fd to standard output (the terminal). This means that any writes to standard output will in fact be sent to the `out.txt` file. You can assume that commands will contain either one input or one output redirection and will not contain both. In other words, this shell is **not** concerned with command sequences such as ```sort < in.txt > out.txt.```

### Communication via a Pipe
The final addition to this shell is allowing the output of one command to serve as input to another using a `pipe`. For example, the following command sequence ```mysh:~$ ls -l | less``` has the output of the command `ls -l` serve as the input to the less command. Both the `ls` and `less` commands run as separate processes and communicate using the UNIX `pipe()` function. Although several commands can be chained together using multiple pipes, this basic shell assumes that commands will contain only one pipe character and will not be combined with any redirection operators.

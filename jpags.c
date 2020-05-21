/**
 * Simple shell interface starter kit program.
 * Operating System Concepts
 * Author: Julia Paglia
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE		80 /* 80 chars per line, per command */

//Function Declarations
char *read_args(void);
char **parse_args(char *myline, char **args);
void is_background_ps(char *line, int *wait);
int execute_args(char *line, char **args, int waitforchild);
int io_redirect(char **args, int *fd);
void is_redirect(char *line, int *io);

int main(void)
{
    char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
    int should_run = 1;

    char *line;
    int waitforchild = 0;
    char *lastcommand = NULL;
    char *lastargs[MAX_LINE/2 + 1];
    
    int io, file, d;
            
    while (should_run){

        fflush(stdout);
        fflush(stdin);
        int mystdout = dup(1);
        int mystdin = dup(0);

        //Read the command(s) passed in by the user
        printf("jpags:~$ ");
        line = read_args();

        //Exit shell        
        if (strcmp(line, "exit\n") == 0) {
            printf("Exiting Shell jpags...Goodbye!\n");
            should_run = 0;
            
            //Kill any remaining processes
            kill(0, SIGINT);
            kill(0, SIGKILL);
            exit(0);
        }
        //History feature with !!
        if (strcmp(line, "!!\n") == 0) {
            if (lastcommand == NULL){
                printf("No commands in history.\n");
                continue;
            }
            else {
                printf("Running previous command: %s\n", lastcommand);
                line = lastcommand;
            }
        }
        else {
            //create a deep copy of the line in memory for history
            if (sizeof(args) > 0 && sizeof(line) > 0) {
                lastcommand = (char*) malloc(MAX_LINE);
                memcpy(lastargs, args, MAX_LINE);
                memcpy(lastcommand, line, MAX_LINE);
            }
        }

        //Check if command is a background process and handle accordingly
        if (strchr(line,'&') != NULL){ 
            is_background_ps(line, &waitforchild);
        }
        else {
            waitforchild = 0;
        }

        //Check if command is a redirect and handle accordingly
        if (strchr(line, '>') != NULL || strchr(line, '<') != NULL) {
            parse_args(line, args);          
            io_redirect(args, &file);
            is_redirect(line, &io);
            parse_args(line, args);
            execute_args(line, args, waitforchild);
        }
        else
        {
            parse_args(line, args);
            execute_args(line, args, waitforchild);
        }
        //Close all open streams for stdin and stdout
        fflush(stdout);
        fflush(stdin);
        dup2(mystdout, 1);
        dup2(mystdin, 0);
        close(mystdout);
        close(mystdin);
    }
    return 0;
}

//Function will read the arguments passed in as user input command
char *read_args(void)
{
    char *argsline = NULL;
    ssize_t buffsize = 0;
    getline(&argsline, &buffsize, stdin);
    return argsline;
}

//Function will set args based on the contents of myline
char **parse_args(char *myline, char **args)
{
    //Create deep copy of *myline
    char *l = (char*) malloc(MAX_LINE);
    memcpy(l, myline, MAX_LINE);
    //Read *line until EOL is reached
    while (*l != '\0'){
        //Iterate through all whitespace chars
        while (*l == ' ' || *l == '\t' || *l == '\n'){
            //Set whitespace to null chars
            *l = '\0';
            *l++;
        }
        //Point to beginning of command and add this to args
        if (*l){
            *args = l;
            *args++;
        }
        //Move pointer forward until whitespace reached
        while (*l != '\0' && *l != ' ' && *l != '\t' && *l != '\n'){
            l++;
        }
    }
    //Set end of args array to EOL
    *args = '\0';
}

//Function will set waitforchild and parse *line for the command before the &
void is_background_ps(char *line, int *wait)
{
    char *begin;
    char *end;
    *wait = 0;
    begin = line;
    end = line;

    while (*begin != '\0'){
        *begin = *end;
        if (*end != '&'){ end++; }
        else { *wait = 1; }
        *begin++;
    }
    *end = '\0';
}

//Function will set io and parse *line for the command before the redirect char (> or <)
void is_redirect(char *line, int *io)
{
    char *begin;
    char *end;
    *io = 0;
    begin = line;
    end = line;

    while (*begin != '\0') {
        *begin = *end;
        if (*end != '>' && *end != '<' && *io == 0) { end++; }
        else { *io = 1; }
        begin++;
    }
    *end = '\0';
}

//Function will handle the redirect output to or from a file
int io_redirect(char **args, int *fd)
{
    int index = 0;
    while (args[index] != NULL){
        //If the command is redirecting an INPUT
        if (strcmp(args[index], "<") == 0){
            *fd = open(args[index+1], O_RDONLY);
            dup2(*fd, 0);
            close(*fd);
        }
        //If the command is redirecting an OUTPUT
        else if (strcmp(args[index], ">") == 0) {
            *fd = open(args[index+1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            dup2(*fd, 1);
            close(*fd);
        }
        index++;
    }
}

//Function will handle the execution of args as shell commands
int execute_args(char *line, char **args, int waitforchild)
{
    pid_t pid;
    int status;

    pid = fork(); //Fork a child

    if (pid < 0) { //Report error for invalid child process
        printf("ERROR: Forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) {  //the child process
        //If command is background process (& included), start new session for the child
        //This will hide the terminal output for background processes
        if (waitforchild == 1) {
            setpgid(0,0);
        }
        //Handle the cd command
        int result = -1;
        if (strcmp(args[0], "cd") == 0){
            if (args[1] == NULL || *args[1] == '~'){
                result = chdir(getenv("HOME"));
            }
            else {
                result = chdir(args[1]);
            }
            if (result < 0) {
                printf("ERROR: Unable to change to directory %s\n", args[1]);
            }
        }
        //Handle pipe command
        else if (strchr(line, '|') != NULL) {
            char *inargs[MAX_LINE / 2 + 1];
            char *outargs[MAX_LINE / 2 + 1];
            char *pipein = strdup(line);
            char *pipeout;
            int p[2];
            int pid2;

            pipein = strtok(line, "|");
            pipeout = strtok(NULL, "|");

            if(pipe(p) < 0){
                printf("ERROR: Pipe failed\n");
                exit(0);
            }
            
            //Fork another child process
            pid2 = fork();
            //Handle the pipe input
            if (pid2 == 0) {
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);
                parse_args(pipein, inargs);
                status = execvp(inargs[0], inargs);
                if (status < 0) {
                    printf("Invalid input command for the pipe\n");
                    exit(0);
                }
            }
            
            pid2 = fork();
            //Execute the output from the pipe input for the command after the pipe
            if (pid2 == 0) {
                close(0);
                dup(p[0]);
                close(p[1]);
                close(p[0]);

                //fflush(stdout);
                parse_args(pipeout, outargs);
                status = execvp(outargs[0], outargs);
                if (status < 0) {
                    printf("Invalid output command after the pipe\n");
                    exit(0);
                }
            }
            //Close all open streams
            close(p[0]);
            close(p[1]);
            wait(NULL);
            wait(NULL);
        }
        else {
            fflush(stdout);
            status = execvp(args[0], args);
            if (status < 0) {
                printf("Invalid command\n");
                exit(0);
            }
        }
    }
    else { //the parent process
        //Parent will wait for child process if '&' has been detected
        //The presence of & is indicated by waitforchild = 1
        int *pidpointer;
        if (waitforchild == 0){
            waitpid(pid, pidpointer, 0);
        }
    }        
    return 0;
}     




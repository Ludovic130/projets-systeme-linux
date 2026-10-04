#include "lib.h"

int cut(char *command, char **argv) {

    int pos = 0; // position of each character string in the argv array
    char *token = strtok(command, " \t\n\r");

    while (token != NULL) {

        argv[pos] = token;
        pos++;
        token = strtok(NULL, " \t\n\r"); // ignore tabs and newlines, replaced by \0 (end of string)
    }
    argv[pos] = NULL; // terminate the argv array with a NULL pointer to indicate the end of the array

    return pos;
}

void red(char **argv, int c) 
{
    for (int i = 0 ; i < c; i++)
    {
        if (strcmp(argv[i], ">") == 0) {
            if (i+1 >= c || argv[i+1] == NULL) {
                fprintf(stderr, "Syntax error: missing file name after '>'\n");
                exit(1);
            }
            int f = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(f == -1)
            {
                perror("File descriptor error");
                exit(1);
            }

            if (dup2(f, STDOUT_FILENO) == -1) // Redirect standard output, if return value is -1
            {
                perror("Redirection error");
                exit(1);
            }

            close(f);
            // Remove '>' and its argument from argv
            argv[i] = NULL;   // cut the list starting from '>'
            break;
        } 
        else if (strcmp(argv[i], "<") == 0) {
            if (i+1 >= c || argv[i+1] == NULL) {
                fprintf(stderr, "Syntax error: missing file name after '<'\n");
                exit(1);
            }
            int f = open(argv[i+1], O_RDONLY, 0644);
            if(f == -1)
            {
                perror("File descriptor error");
                exit(1);
            }
            if (dup2(f, STDIN_FILENO) == -1) // Redirect standard input, if return value is -1
            {
                perror("Redirection error");
                exit(1);
            }
            close(f);
            argv[i] = NULL;   // cut the list starting from '<'
            break;
        } 
        else if (strcmp(argv[i], ">>") == 0) {
            if (i+1 >= c || argv[i+1] == NULL) {
                fprintf(stderr, "Syntax error: missing file name after '>>'\n");
                exit(1);
            }
            int f = open(argv[i+1], O_RDWR | O_CREAT | O_APPEND, 0644);
            if(f == -1)
            {
                perror("File descriptor error");
                exit(1);
            }
            if (dup2(f, STDOUT_FILENO) == -1) // Redirect standard output, if return value is -1
            {
                perror("Redirection error");
                exit(1);
            }
            close(f);
            argv[i] = NULL;   // cut the list starting from '>>'
            break;
        }
    }
}

void cmd(char **argv, int c) 
{
    __pid_t pid; // Create variable to store child PID

    do {
        pid = fork(); // fork() may fail temporarily due to insufficient resources, so it is advisable to retry in that case
    } while ((pid == -1) && (errno == EAGAIN)); // EAGAIN: Temporary resource shortage to create a child process, retry   

    if (pid == -1) // fork() failed after several attempts
    {
        fprintf(stderr, "Process forking error\n");
        exit(1);
    }

    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        red(argv, c);
        if(execvp(argv[0], argv) == -1) { // execvp() returns -1 on error, and errno is set to indicate the specific error
            perror("Error executing command"); 
            exit(1);
        }
    } else {
        waitpid(pid, NULL, 0); // wait for the child process to finish before continuing
    }

}

void chdr(char **argv, char *rep)
{
    if (argv[1] != NULL)
    {
        if(chdir(argv[1]) == 0) // Change the current directory with chdir()
        {
            // Get the current directory
            if ((rep != NULL) && getcwd(rep, 1024) == NULL) // if NULL, display error
            {
                perror("Failure");
            }
        } else { // if chdir returns -1, display error
            fprintf(stderr, "Directory change error\n"); 
        }
    } else { // otherwise if argv[1] == NULL 
        if(chdir("/root") == 0) // change to root directory with chdir()
        {
            if (getcwd(rep, 1024) == NULL) // get the directory; if NULL, display error
            {
                fprintf(stderr, "Failed to get current directory\n");
            }
        } else {
            fprintf(stderr, "Directory change error\n");
        }
    }
}

void choose(char **argv, char *rep, int c)
{

    if((argv[0] != NULL) && (strcmp(argv[0],"cd") == 0)) 
    {
        chdr(argv, rep);
    } 
    else if ((argv[0] != NULL) && (strcmp(argv[0],"exit") == 0)) // if the command is exit then
    {
        exit(0); // exit
    } else if (argv[0] != NULL) 
    {
        cmd(argv, c); // otherwise execute this function if none of the above cases are valid.
    }

}

int main(void)
{
    char *line;
    char *argv[20]; // declare an array of pointers of size 20
    char prompt[300];
    char rep[1024];
    
    if(signal(SIGINT, SIG_IGN) == SIG_ERR)  // Ignore SIGINT signal when pressing Ctrl+C
    {
        fprintf(stderr, "signal not caught");
    }

    while (1)
    {
        if (getcwd(rep, 1024) != NULL)
        {
            int s = snprintf(prompt, sizeof(prompt), "Ludovic:~%s> ", rep); // format multiple character strings
            if (s) {
                line = readline(prompt); // read the command entered by the user
            }
        } 
        
        if (!line) {
            break;
        }
        
        if(*line){
            add_history(line); // add history to allow up/down arrow navigation to previously typed text.
                               // previously typed.
        }
        int c = cut(line, argv);
        choose(argv, rep, c);
        
        free(line); // free memory allocated by readline()
    }

    return 0;
}
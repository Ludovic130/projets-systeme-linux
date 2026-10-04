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

// Apply redirections (>, >>, <) on a single command (sub-list)
void apply_redir_to_cmd(char **cmd)
{
    int i = 0;
    while (cmd[i] != NULL)
    {
        if (strcmp(cmd[i], ">") == 0) {
            if (cmd[i+1] == NULL) {
                fprintf(stderr, "Syntax error: missing file name after '>'\n");
                exit(1);
            }
            int f = open(cmd[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (f == -1) { perror("open >"); exit(1); }
            if (dup2(f, STDOUT_FILENO) == -1) { perror("dup2 >"); exit(1); }
            close(f);
            cmd[i] = NULL; // cut the list at the redirection symbol
            break;
        } else if (strcmp(cmd[i], ">>") == 0) {
            if (cmd[i+1] == NULL) {
                fprintf(stderr, "Syntax error: missing file name after '>>'\n");
                exit(1);
            }
            int f = open(cmd[i+1], O_RDWR | O_CREAT | O_APPEND, 0644);
            if (f == -1) { perror("open >>"); exit(1); }
            if (dup2(f, STDOUT_FILENO) == -1) { perror("dup2 >>"); exit(1); }
            close(f);
            cmd[i] = NULL;
            break;
        } else if (strcmp(cmd[i], "<") == 0) {
            if (cmd[i+1] == NULL) {
                fprintf(stderr, "Syntax error: missing file name after '<'\n");
                exit(1);
            }
            int f = open(cmd[i+1], O_RDONLY, 0644);
            if (f == -1) { perror("open <"); exit(1); }
            if (dup2(f, STDIN_FILENO) == -1) { perror("dup2 <"); exit(1); }
            close(f);
            cmd[i] = NULL;
            break;
        }
        i++;
    }
}

void exec_pfils(int nb_cmd, int c, char **argv)
{
    char **ls_cmd[nb_cmd]; // Create grouped commands split after |
    int index = 0;
    ls_cmd[0] = argv; // position the first command at argv[0]

    for (int i = 0; i < c; i++)
    {
        if (strcmp(argv[i], "|") == 0) 
        {
            argv[i] = NULL; // Set argv[i] containing "|" to NULL
            index++;
            ls_cmd[index] = &argv[i+1];
        }
    }

    int num_pipes = nb_cmd - 1;
    // If num_pipes > 0 then tubes[num_pipes][2] otherwise tubes[1][2];
    int tubes[num_pipes > 0 ? num_pipes : 1][2];
    for (int i = 0; i < num_pipes; i++)
    {
        if(pipe(tubes[i]) != 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid;

    for (int i = 0; i < nb_cmd; i++)
    {
        pid = fork();

        if(pid == -1) // If pid returns -1, send a message and exit abruptly.
        {
            perror("fork()");
            exit(1);
        } 

        if (pid == 0)
        {
            // Redirect standard output (ignore the last command).
            if (i < nb_cmd - 1)
            {
                if (dup2(tubes[i][1], STDOUT_FILENO) == -1) // duplicate the pipe input to standard output
                {
                    perror("dup2 stdout");
                    exit(1);
                }
            }

            // Redirect standard input (ignore the first command).
            if (i > 0)
            {
                if(dup2(tubes[i-1][0], STDIN_FILENO) == -1) // duplicate the pipe output to standard input
                {
                    perror("dup2 stdin");
                    exit(1);
                }
            }

            // Close all pipe descriptors in this child
            for (int j = 0; j < num_pipes; j++)
            {
                close(tubes[j][0]);
                close(tubes[j][1]);
            }

            // Apply redirections (>, >>, <) on this specific command
            apply_redir_to_cmd(ls_cmd[i]);

            // Execute the command 
            if(execvp(ls_cmd[i][0], ls_cmd[i]) == -1)
            {
                perror("execvp");
                exit(1);
            }
        }
    }

    // Also close all descriptors in the parent
    for (int i = 0; i < num_pipes; i++)
    {
        close(tubes[i][0]);
        close(tubes[i][1]);
    }

    // Wait for all children to finish
    for (int i = 0; i < nb_cmd; i++)
    {
        if(wait(NULL) == -1)
        {
            perror("wait");
            exit(1);
        }
    }
}

void exec_pipe(char **argv, int c)
{
    int nb_cmd = 1;
    for (int i = 0; i < c; i++)
    {
        if (strcmp(argv[i], "|") == 0) // If argv[i] contains "|" which equals "|"
        {
            nb_cmd++; // Add +1 to nb_cmd
        }
    }

    exec_pfils(nb_cmd, c, argv);
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
            if(f == -1) // If it returns a negative value instead of a descriptor
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
            argv[i] = NULL; // Set the previous character (which is ">") to NULL.

            break; // Exit the loop since redirection is done.
        } else if (strcmp(argv[i], "<") == 0) {
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
            argv[i] = NULL; // Set the previous character (which is "<") to NULL.

            break; // Exit the loop since redirection is done.
        } else if (strcmp(argv[i], ">>") == 0) {
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
            argv[i] = NULL; // Set the previous character (which is ">>") to NULL.

            break; // Exit the loop since redirection is done.
        }
    }
}

void cmd(char **argv, int c) 
{
    // Check if there is a pipe symbol
    int has_pipe = 0;
    for (int i = 0; i < c; i++) {
        if (strcmp(argv[i], "|") == 0) {
            has_pipe = 1;
            break;
        }
    }

    if (has_pipe) {
        exec_pipe(argv, c); // exec_pipe handles forking internally
        return;
    }

    // Otherwise, simple command with optional redirection
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
        red(argv, c); // apply simple redirections
        if(execvp(argv[0], argv) == -1) { // execvp() returns -1 on error, and errno is set to indicate the specific error
            perror("Error executing command"); 
            exit(1);
        }
    } else {
        if(waitpid(pid, NULL, 0) == -1) // wait for the child process to finish before continuing
        {
            perror("waitpid");
            exit(1);
        }
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

// 4
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
            int s = snprintf(prompt, sizeof(prompt), "Ludovic:\n~%s> ", rep); // format multiple character strings
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
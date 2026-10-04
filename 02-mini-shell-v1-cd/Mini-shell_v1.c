#include "lib.h"

void cut(char *command, char **argv) {

    int pos = 0; // position of each character string in the argv array
    char *token = strtok(command, " \t\n\r");

    // Limit to 19 tokens to avoid overflow (argv[20] max, last is NULL)
    while (token != NULL && pos < 19) {

        argv[pos] = token;
        pos++;
        token = strtok(NULL, " \t\n\r"); // ignore tabs and newlines, replaced by \0 (end of string)
    }
    argv[pos] = NULL; // terminate the argv array with a NULL pointer to indicate the end of the array

}

void cmd(char **argv) 
{
    __pid_t pid;

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
        if(execvp(argv[0], argv) == -1) { // execvp() returns -1 on error, and errno is set to indicate the specific error
            perror("Error executing command"); 
            exit(1);
        }
    } else {
        waitpid(pid, NULL, 0); // wait for the child process to finish before continuing
    }
}

int main(void)
{
    char *line = NULL;
    char *argv[20];
    char prompt[1024];
    char rep[300];
    
    if(signal(SIGINT, SIG_IGN) == SIG_ERR) 
    {
        fprintf(stderr, "signal not caught");
    }

    while (1)
    {
        line = NULL;

        if (getcwd(rep, sizeof(rep)) != NULL)
        {
            int s = snprintf(prompt, sizeof(prompt), "Ludovic:\n~%s> ", rep);
            if (s) {
                line = readline(prompt); // read the command entered by the user
            }
        } 
        
        if (!line) {
            break; // si readline a retourné NULL (EOF ou échec), on quitte
        }
        if(*line){
            add_history(line); // add history to allow up/down arrow navigation to previously typed text.
        }

        // If the line contains only spaces or is empty, skip processing
        // but still free it (we do it at the end).
        // We can just let cut handle it, but cut will set argv[0]=NULL.
        cut(line, argv);

        // --- FIX: always check argv[0] != NULL before using strcmp ---
        if (argv[0] != NULL && strcmp(argv[0], "cd") == 0) 
        {
           
            if (argv[1] != NULL)
            {
                if(chdir(argv[1]) == 0) // Change the current directory with chdir()
                {
                    // Get the current directory
                    if (getcwd(rep, sizeof(rep)) == NULL) // if NULL, display error
                    {
                        fprintf(stderr, "Failed to get current directory");
                    }
                } else { // if chdir returns -1, display error
                    fprintf(stderr, "Directory change error"); 
                }
            } else { // otherwise if argv[1] == NULL 
                if(chdir("/root") == 0) // change to root directory
                {
                    if (getcwd(rep, sizeof(rep)) == NULL) // get the directory; if NULL, display error
                    {
                        fprintf(stderr, "Failed to get current directory");
                    } else {
                        snprintf(prompt, sizeof(prompt),"%s", rep);
                    }
                } else {
                    fprintf(stderr, "Directory change error");
                }
            }
        } 
        else if (argv[0] != NULL && strcmp(argv[0], "exit") == 0) // if the command is exit then 
        {
            exit(0); // exit
        } 
        else if (argv[0] != NULL) 
        {
            cmd(argv);
        }
        
        free(line);
    }
    
    return 0;
}
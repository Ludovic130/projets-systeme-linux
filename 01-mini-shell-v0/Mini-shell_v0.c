#include "lib.h"

int main(void)
{
    // issue encountered:
    /*  argv is a variable of type array of pointers to char. To use it with keyboard input, we must define
        its size, then assign the command entered at the keyboard, and it must always end with a NULL pointer.
        example: argv[0] = command;  // or argv[0] = &command[0];  // or argv[0] = (char *)command;
                 argv[1] = NULL;

        NB: Bad address error: occurs when the address of the entered command is not correctly referenced in the argv array,
            which can lead to an attempt to access an invalid memory address when executing the command with execvp().
    */
    
    // Read a string
    // read the line
    // execute the command

    char command[10];

    int pos = 0; // position of each character string in the argv array

    printf("root@mini-shell:~$ ");
 
    fgets(command, sizeof(command), stdin); // read a command entered by the user

    char *argv[10];
    char *token = strtok(command, " \t\n");


    while (token != NULL) {

        argv[pos] = token;
        pos++;
        token = strtok(NULL, " \t\n"); // ignore tabs and newlines, replaced by \0 (end of string)
    }
    argv[pos] = NULL; // terminate the argv array with a NULL pointer to indicate the end of the array

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
        wait(NULL); // wait for the child process to finish before continuing
    }
    
    return 0;
}
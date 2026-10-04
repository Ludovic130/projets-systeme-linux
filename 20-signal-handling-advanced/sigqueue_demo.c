#include "lib.h"

void syntaxe (const char * nom) // Function that displays the argument entered in the shell
{
    fprintf(stderr,"syntax %s signal pid...\n", nom);
    exit(EXIT_FAILURE);
}

int main(int argc, char * argv[])
{
    int i;
    int numero;
    int pid;
    union sigval valeur;
    

    if(argc == 1) // If there is only 1 argument, display the argument
    {
        syntaxe(argv[0]);
    }

    i = 1;

    if(argc == 2) 
    {   
        numero = SIGTERM; // Signal to gracefully terminate the process
    } else {
        if(sscanf(argv[i], "%d", &numero) != 1) // argv[1] will be stored at the address of numero
        {
            syntaxe(argv[0]);
        }
        i++; // i = 1 + 1 = 2
    }

    if((numero < 0) || (numero > _NSIG - 1)) // If the number is less than 0 or greater than 32, call the function
    {
        syntaxe(argv[0]);
    }
    valeur.sival_int = 0; // sival_int takes the default value 0

    for (; i < argc; i++)
    {
        if(sscanf(argv[i], "%d", &pid) != 1) // argv[2] will be the PID of the process
        {
            syntaxe(argv[0]);
        }
        if (sigqueue((pid_t)pid, numero, valeur) < 0) // Send the additional signal information to the handler
        {
            fprintf(stderr, "%d: ",pid);
            perror("sigqueue");
        }
    }
    

    return 0;
}
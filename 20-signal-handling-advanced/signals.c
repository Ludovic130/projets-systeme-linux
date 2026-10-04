#include "lib.h"

void gestionnaire(int numero, siginfo_t *info, void *inutile) // Signal handler
{
    fprintf(stderr, "Received %d\n", numero);
    fprintf(stderr, "si_code = %d\n", info->si_code);
}

int main(void)
{
    int i;
    struct sigaction action; // Structure to define the signal action
    char chaine[5]; // String to read the user's input

    action.sa_sigaction = gestionnaire; // We set the signal handler
    action.sa_flags = SA_SIGINFO; // We want additional information about the signal
    sigemptyset(&action.sa_mask); // We do not block any signal while the handler runs
    
    fprintf(stderr, "PID=%ld\n", (long) getpid()); // Print the process PID

    for (i = 0; i < _NSIG; i++)
    {
        if(sigaction(i, &action, NULL) < 0) // Try to intercept signal i
        {
            fprintf(stderr,"%d not intercepted\n", i);
        }
        
    }

    while (1)
    {
        fgets(chaine, 5, stdin); // Wait for user input to continue
    }
    
    return EXIT_SUCCESS;
}
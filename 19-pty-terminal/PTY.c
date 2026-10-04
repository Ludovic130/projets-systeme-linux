#include "lib.h"

void gestionnaire(int numero)
{
    exit(EXIT_SUCCESS);
}

void copie_entrees_sorties(int fd, int sock)
{
    int max;
    fd_set set;
    char buffer[4096];
    int nb_lus;

    max = sock < fd ? fd : sock;

    while (1)
    {
        FD_ZERO(&set);
        FD_SET(sock, &set);
        FD_SET(fd, &set);

        if(select(max + 1, &set, NULL, NULL, NULL) < 0) // Stays blocked as long as no data arrives on the monitored descriptors
        {
            break;
        }

        if(FD_ISSET(sock, &set)) // If the check is true, the socket has indeed received data from e.g. the client
        {
            if((nb_lus = read(sock, buffer, 4096)) >= 0) // read the data from the socket to carry it into the buffer
            {
                write(fd, buffer, nb_lus); /* Write the data contained in the buffer to the master pseudo-terminal descriptor
                                              which is linked to the slave pseudo-terminal which in turn is linked to the /bin/sh shell
                                            */
            }
            else
            {
                break;
            }
        }

        if(FD_ISSET(fd, &set)) 
        {
            if((nb_lus = read(fd, buffer, 4096)) >= 0)
            {
                write(sock, buffer, nb_lus);
            }
            else
            {
                break;
            }
        }
    }  
}

void traite_connexion(int sock)
{
    int fd_maitre;
    int fd_esclave;
    struct termios termios_stdin;
    struct termios termios_maitre;
    char *args[2] = {"/bin/sh", "-i", NULL};
    char *nom_esclave;

    if((fd_maitre = getpt()) < 0) // Create a pseudo-terminal
    {
        perror("no Unix 98 Pseudo TTY available \n");
        exit(EXIT_FAILURE);
    }
    grantpt(fd_maitre); // Modify access rights to the slave pseudo-terminal
    unlockpt(fd_maitre); // Unlock the slave pseudo-terminal
    nom_esclave = ptsname(fd_maitre);
    tcgetattr(STDIN_FILENO, &termios_stdin); // Get the current terminal configuration

    switch (fork())
    {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            close(fd_maitre);
            /* Detach from the previous controlling terminal */
            setsid(); // Detach the process from the terminal

            /* Open the slave pseudo-terminal which becomes */
            /* Then the controlling terminal of this process. */
            if((fd_esclave = open(nom_esclave, O_RDWR)) < 0)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            tcsetattr(fd_esclave, TCSANOW, &termios_stdin);
            dup2(fd_esclave, STDIN_FILENO);
            dup2(fd_esclave, STDOUT_FILENO);
            dup2(fd_esclave, STDERR_FILENO);
            execv(args[0], args);
            break;
        default:
            tcgetattr(fd_maitre, &termios_maitre); // Get the parameters of the master pseudo-terminal
            cfmakeraw(&termios_maitre); // Raw mode (no processing on standard input and output)
            tcsetattr(fd_maitre, TCSANOW, &termios_maitre); // Apply the changes to the master pseudo-terminal
            copie_entrees_sorties(fd_maitre, sock);
            exit(EXIT_SUCCESS);
    }
}

int main(void)
{
    int sock;
    int sock_2;
    struct sockaddr_in adresse;
    socklen_t longueur;

    if(signal(SIGINT, gestionnaire) != 0)
    {
        perror("signal");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    memset(&adresse, 0, sizeof(struct sockaddr)); // set the structure to 0
    adresse.sin_family = AF_INET;
    adresse.sin_addr.s_addr = htonl(INADDR_ANY);
    adresse.sin_port = 0;
    
    if(bind(sock, (struct sockaddr *)&adresse, sizeof(adresse)) < 0) // give it an identity
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    longueur = sizeof(struct sockaddr_in);

    if(getsockname(sock, (struct sockaddr *)&adresse, &longueur) < 0)
    {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "My address: IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));

    listen(sock, 5);

    while (1)
    {
        longueur = sizeof(struct sockaddr_in);
        sock_2 = accept(sock, (struct sockaddr *)&adresse, &longueur);
        if(sock_2 < 0)
        {
            continue;
        }
        switch (fork())
        {
            case 0:
                close(sock); // Close the connection socket
                traite_connexion(sock_2);
                exit(EXIT_SUCCESS);
            default:
                close(sock_2); // Close the accepted connection socket
                break;
        }
    }
    close(sock); // Close the socket once done

    return EXIT_SUCCESS;
}
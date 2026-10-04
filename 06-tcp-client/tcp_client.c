#include "lib.h"

int lecture_arguments(int argc, char *argv[], struct sockaddr_in *adresse, char *protocole)
{
    char *liste_options = "a:p:h"; // list of possible options ((-a)>address (:)>argument, (-p)>port (:)>argument, (-h)>help (:)>argument)
    int options;
    char *hote = "localhost";
    char *port = "2000";
    struct hostent *hostent;
    struct servent *servent;
    int numero;

    // Loop for handling command-line arguments.
    while ((options = getopt(argc, argv, liste_options)) != -1) // getopt() allows iterating over command-line options
    {
        switch (options)
        {
        case 'a':
            hote = optarg; // Global constant that contains the argument (argv[]) of the -a option
            break;
        case 'p':
            port = optarg; // Global constant that contains the argument (argv[]) of the -p option
            break;
        case 'h':
            fprintf(stderr, "Syntax: %s [-a address] [-p port] \n", argv[0]);
            return -1;
        default:
            break;
        }
    }
    memset(adresse, 0, sizeof(struct sockaddr_in));

    // inet_aton() converts an IPv4 address in dotted notation into an in_addr structure. It returns 0 if the address is invalid, otherwise it returns a non-zero number.
    if (inet_aton(hote, &(adresse->sin_addr)) == 0)
    {
        if ((hostent = gethostbyname(hote)) == NULL) // Convert a hostname into an IP address. It returns NULL if the hostname is unknown.
        {
            fprintf(stderr, "host = %s unknown \n", hote);
            return -1;
        }
        adresse->sin_addr.s_addr = ((struct in_addr *)(hostent->h_addr))->s_addr;
    }

    if (sscanf(port, "%d", &numero) == 1) // sscanf() converts a string into an integer. It returns the number of items converted and assigned. If the port is a number, we convert it to an integer and store it in the sockaddr_in structure
    {
        adresse->sin_port = htons(numero);
        return 0;
    }

    if ((servent = getservbyname(port, protocole)) == NULL) // getservbyname() converts a service name into a port number.
    {
        fprintf(stderr, "Service %s unknown \n", port);
    }

    adresse->sin_port = servent->s_port;

    return 0;
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in adresse;
    char buffer[LG_BUFFER];
    int nb_lus;

    if (lecture_arguments(argc, argv, &adresse, "tcp") < 0) // Reading command-line arguments. If reading fails, we exit the program
    {
        exit(EXIT_FAILURE);
    }
    adresse.sin_family = AF_INET;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // socket() creates a socket. It returns -1 if creation fails, otherwise it returns a file descriptor for the created socket.
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&adresse, sizeof(struct sockaddr_in)) < 0) // connect() establishes a connection with the server. It returns -1 if the connection fails, otherwise it returns 0.
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    // setvbuf(stdout, NULL, _IONBF, 0); // Allows acting on a buffer depending on the chosen mode. Here, we choose _IONBF mode (no buffer) so that outputs are displayed immediately.

    while (1)
    {
        if ((nb_lus = read(sock, buffer, LG_BUFFER)) == 0) // If the server closed the connection, we exit the loop
        {
            break;
        }

        if (nb_lus < 0) // If the read failed, we display an error message and exit the program
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        write(STDOUT_FILENO, buffer, nb_lus); // Display the message received from the server on standard output
    }

    return EXIT_SUCCESS;
}
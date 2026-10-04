#include "lib.h"

int creat_socket_stream(const char *nom_hôte, const char *nom_service, const char *nom_proto)
{
    int sock;
    struct sockaddr_in adresse;
    struct hostent *hostent;
    struct servent *servent;
    struct protoent *protoent;

    memset(&adresse, 0, sizeof(struct sockaddr_in)); // Point to the beginning of the sockaddr_in structure to modify
    adresse.sin_family = AF_INET;

    if (nom_hôte == NULL) // Check if the nom_hôte pointer is NULL; if so, use the local machine's IP address
    {
        adresse.sin_addr.s_addr = htonl(INADDR_ANY); // IP address of the local machine
    }
    else
    {
        // Host resolution
        hostent = gethostbyname(nom_hôte); // Convert a hostname into an IP address
        if (hostent == NULL)               // If resolution fails
        {
            perror("gethostbyname");
            return -1;
        }
        adresse.sin_addr.s_addr = ((struct in_addr *)hostent->h_addr_list)->s_addr; // Store the IP address in the sockaddr_in structure
    }

    // Protocol information
    if ((protoent = getprotobyname(nom_proto)) == NULL) // Retrieve the official name of a network protocol from its name
    {
        perror("getprotobyname");
        return -1;
    }

    if (nom_service == NULL) // Check if the nom_service pointer is NULL; if so, use a dynamic port
    {
        adresse.sin_port = htons(0); // Dynamic port
    }
    else
    {
        // Service information
        if ((servent = getservbyname(nom_service, protoent->p_name)) == NULL) // Retrieve the name of a network service from its name and protocol
        {
            perror("getservbyname");
            return -1;
        }
        adresse.sin_port = servent->s_port; // The service port is stored in the sockaddr_in structure
    }

    // Socket creation
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Create a TCP socket with domain AF_INET, type SOCK_STREAM and default protocol (0)
    {
        perror("socket");
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&adresse, sizeof(struct sockaddr_in)) < 0) // Associate the socket with the address and port specified in the sockaddr_in structure
    {
        close(sock);
        perror("bind");
        return -1;
    }

    return sock; // return the descriptor of the socket created and associated with the specified address and port
}

int affiche_adresse_socket(int sock)
{
    struct sockaddr_in adresse; // Structure
    socklen_t longueur;

    longueur = sizeof(struct sockaddr_in);

    if (getsockname(sock, (struct sockaddr *)&adresse, &longueur) < 0) // Get the address and port of the socket
    {
        perror("getsockname");
        return -1;
    }

    fprintf(stdout, "IP= %s, Port: %u \n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port)); // Display the IP address and port of the socket on standard output

    return 0;
}

void traite_connexion(int sock)
{
    struct sockaddr_in adresse;
    socklen_t longueur;
    char buffer[256];

    longueur = sizeof(struct sockaddr_in);

    if ((getpeername(sock, (struct sockaddr *)&adresse, &longueur)) < 0) // Get the address and port of the client connected to the socket
    {
        perror("getpeername");
        return;
    }

    sprintf(buffer, "IP = %s, Port = %u \n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port)); // the address and port of the connected client are stored in the buffer

    fprintf(stdout, "Local connection: ");
    affiche_adresse_socket(sock);            // Display the address and port of the local socket on standard output
    fprintf(stdout, "Remote: %s", buffer);   // Display the address and port of the connected client on standard output

    write(sock, "Your address: ", 14);
    write(sock, buffer, strlen(buffer));

    send(sock, "Hello \n", 8, 0); // Send a "Hello" message to the client connected to the socket

    close(sock); // WARNING: Close the socket after handling the connection, otherwise the client will not receive the message
}

int server_tcp(void)
{
    int sock_attente_connexion;
    int sock_connectee;
    struct sockaddr_in adresse;
    socklen_t longueur;

    // int quitter = 1;

    sock_attente_connexion = creat_socket_stream(NULL, NULL, "tcp"); // Store the descriptor of the created socket in the variable sock_attente_connexion

    if (sock_attente_connexion < 0) // If socket creation failed, display an error message and return -1
    {
        perror("creat_socket_stream");
        return -1;
    }

    listen(sock_attente_connexion, 5); // Put the socket in listening mode to accept incoming connections, with a queue of up to 5 connections

    fprintf(stdout, "My address >> ");
    affiche_adresse_socket(sock_attente_connexion); // Display the address and port of the listening socket on standard output

    signal(SIGCHLD, SIG_IGN); // Ignore SIGCHLD signals to avoid zombie processes

    while (1)
    {
        longueur = sizeof(struct sockaddr_in);
        sock_connectee = accept(sock_attente_connexion, (struct sockaddr *)&adresse, &longueur); // Accept an incoming connection on the listening socket and return a new socket descriptor for the established connection

        if (sock_connectee < 0) // If accepting the connection failed, display an error message and return -1
        {
            perror("accept");
            return -1;
        }

        switch (fork())
        {
        case -1:
            perror("fork()");
            exit(EXIT_FAILURE);
        case 0: // Child process
            close(sock_attente_connexion);
            traite_connexion(sock_connectee); // Handle the incoming connection on the connected socket
            exit(EXIT_SUCCESS);
        default:                   // Parent process
            close(sock_connectee); // Close the connected socket descriptor in the parent process, since it does not need it
        }
    }

    return 0;
}

int main(void)
{
    return server_tcp(); // Return the result of the server_tcp() function that handles the TCP server
}
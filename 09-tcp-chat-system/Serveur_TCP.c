#include "lib.h"

int creat_socket_stream(const char *nom_hôte, const char *nom_service, const char *nom_proto)
{
    int sock;
    struct sockaddr_in adresse;
    struct hostent *hostent;
    struct servent *servent;
    struct protoent *protoent;

    memset(&adresse, 0, sizeof(struct sockaddr_in)); // Point vers le debut de la structure sockaddr_in a modifier
    adresse.sin_family = AF_INET;                    //

    if (nom_hôte == NULL) // Verifier si le pointeur nom_hôte est NULL, si c'est le cas, on utilise l'adresse IP de la machine locale
    {
        adresse.sin_addr.s_addr = htonl(INADDR_ANY); // Adresse IP de la machine locale
    }
    else
    {
        // Résolution d'hôte
        hostent = gethostbyname(nom_hôte); // Convertir un nom d'hote en adresse IPf
        if (hostent == NULL)               // Si la résolution échoue
        {
            perror("gethostbyname");
            return -1;
        }
        adresse.sin_addr.s_addr = ((struct in_addr *)hostent->h_addr_list)->s_addr; // Stocke l'adresse IP dans la structure sockaddr_in
    }

    // Informations sur les protocoles
    if ((protoent = getprotobyname(nom_proto)) == NULL) // Récupère le nom officiel d'un protocole réseaux à partir de son numéro
    {
        perror("getprotobynumber");
        return -1;
    }

    if (nom_service == NULL) // Vérifier si le pointeur nom_service est NULL, si c'est le cas, on utilise un port dynamique
    {
        adresse.sin_port = htons(0); // Port dynamique
    }
    else
    {
        // Informations sur les services
        if ((servent = getservbyname(nom_service, protoent->p_name)) == NULL) // Récupère le nom d'un service réseau à partir de son numéro de port et de son protocole
        {
            perror("getservbyport");
            return -1;
        }
        adresse.sin_port = servent->s_port; // Le port du service est stocké dans la structure sockaddr_in
    }

    // Création de la socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Création d'une socket TCP avec le domaine AF_INET, le type SOCK_STREAM et le protocole par défaut (0)
    {
        perror("socket");
        return -1;
    }

    if (bind(sock, (struct sockaddr *)&adresse, sizeof(struct sockaddr_in)) < 0) // Associe la socket à l'adresse et au port spécifiés dans la structure sockaddr_in
    {
        close(sock);
        perror("bind");
        return -1;
    }

    return sock; // retourne le descripteur de la socket créée et associée à l'adresse et au port spécifiés
}

int affiche_adresse_socket(int sock)
{
    struct sockaddr_in adresse; // Structure
    socklen_t longueur;

    longueur = sizeof(struct sockaddr_in);

    if (getsockname(sock, (struct sockaddr *)&adresse, &longueur) < 0) // Obtenir l'adresse et le port de la socket
    {
        perror("getsockname");
        return -1;
    }

    fprintf(stdout, "IP= %s, Port: %u \n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port)); // Affiche l'adresse IP et le port de la socket sur la sortie standard

    return 0;
}

void traite_connexion(int sock)
{
    struct sockaddr_in adresse;
    socklen_t longueur;
    char buffer[256];
    char n[LG_BUFFER];

    longueur = sizeof(struct sockaddr_in);

    if ((getpeername(sock, (struct sockaddr *)&adresse, &longueur)) < 0) // Obtenir l'adresse et le port du client connecté à la socket
    {
        perror("getpeername");
        return;
    }

    sprintf(buffer, "IP = %s, Port = %u \n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port)); // l'adresse et le port du client connecté sont stockés dans le buffer

    fprintf(stdout, "Connexion local : ");
    affiche_adresse_socket(sock);            // Affiche l'adresse et le port de la socket locale sur la sortie standard
    fprintf(stdout, "Distant : %s", buffer); // Affiche l'adresse et le port du client connecté sur la sortie standard

    write(sock, "Votre adresse : ", 16); // On ecris les données dans le socket en direction du tube
    write(sock, buffer, strlen(buffer)); // De meme ici

    if (read(sock, n, 3) < 0) // On lit les données envoyer par le client
    {
        perror("send3");
        return;
    }

    write(STDOUT_FILENO, n, 3);
}

int run_select_loop(int sock_attente_connexion)
{
    struct sockaddr_in adresse;
    socklen_t longueur;
    int sock_connectee;
    fd_set readfds;
    int fd_max;

    fd_max = sock_attente_connexion;          // Initialise le descripteur de fichier maximum à surveiller avec la valeur de la socket d'attente de connexion
    FD_ZERO(&readfds);                        // Initialise l'ensemble de descripteurs de fichiers à surveiller
    FD_SET(sock_attente_connexion, &readfds); // Ajoute le descripteur de la socket connectée à l'ensemble de descripteurs de fichiers à surveiller

    while (1)
    {
        longueur = sizeof(struct sockaddr_in);

        fd_set tmp_fds = readfds; // copie avant select

        if (select(fd_max + 1, &tmp_fds, NULL, NULL, NULL) < 0) // Surveille la socket connectée pour voir si elle est prête à être lue
        {
            perror("select");
            return -1;
        }

        if (FD_ISSET(sock_attente_connexion, &tmp_fds)) // Vérifie si la socket d'attente de connexion est prête à être lue
        {
            sock_connectee = accept(sock_attente_connexion, (struct sockaddr *)&adresse, &longueur); // Accepte une connexion entrante sur la socket d'attente de connexion et retourne un nouveau descripteur de socket pour la connexion établie
            FD_SET(sock_connectee, &readfds);                                                        // Ajoute le descripteur du socket au tableau de bit
            if ((sock_connectee) < 0)                                                                // Si l'acceptation de la connexion a échoué, afficher un message d'erreur et retourner -1
            {
                perror("accept");
                return -1;
            }
            if (sock_connectee > fd_max) // Si le descripteur du socket est supérieur a fd_max alors on change la valeur de fd_max
            {
                fd_max = sock_connectee;
            }
        }

        for (int i = 0; i <= fd_max; i++)
        {
            if (i != sock_attente_connexion && FD_ISSET(i, &tmp_fds)) // Vérifie si le descripteur de fichier i est prêt à être lu
            {
                traite_connexion(i); // Traite la connexion entrante sur le descripteur de fichier
                // 3. Maintenant on peut fermer la socket proprement
                if (signal(SIGINT, SIG_DFL))
                {
                    close(i); // Fermer les descripteurs avec le signal Ctrl+C
                    FD_CLR(i, &readfds);
                }
            }
        }
    }
    return 0;
}

int server_tcp(void)
{
    int sock_attente_connexion;

    // int quitter = 1;

    sock_attente_connexion = creat_socket_stream(NULL, NULL, "tcp"); // Stocker le descripteur de la socket créée dans la variable sock_attente_connexion

    if (sock_attente_connexion < 0) // Si la création de la socket a échoué, afficher un message d'erreur et retourner -1
    {
        perror("creat_socket_stream");
        return -1;
    }

    listen(sock_attente_connexion, 5); // Mettre la socket en mode écoute pour accepter les connexions entrantes, avec une file d'attente de 5 connexions maximum

    fprintf(stdout, "Mon adresse >> ");
    affiche_adresse_socket(sock_attente_connexion); // Affiche l'adresse et le port de la socket d'attente de connexion sur la sortie standard

    run_select_loop(sock_attente_connexion); // Executer select()

    return 0;
}

int main(void)
{
    return server_tcp(); // Retourne le résultat de la fonction server_tcp() qui gère le serveur TCP
}

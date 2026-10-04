#include "lib.h"

int lecture_arguments(int argc, char *argv[], struct sockaddr_in *adresse, char *protocole)
{
    char *liste_options = "a:p:h"; // liste des options possibles ((-a)>adresse (:)>argument, (-p)>port (:)>argument, (-h)>aide (:)>argument)
    int options;
    char *hote = "localhost";
    char *port = "2000";
    struct hostent *hostent;
    struct servent *servent;
    int numero;

    // Boucle pour la gestion des arguments de la ligne de commande.
    while ((options = getopt(argc, argv, liste_options)) != -1) // getopt() permet de parcourir les options de la ligne de commande
    {
        switch (options)
        {
        case 'a':
            hote = optarg; // Constante globale qui contient l'argument(argv[]) de l'option -a
            break;
        case 'p':
            port = optarg; // Constante globale qui contient l'argument(argv[]) de l'option -p
            break;
        case 'h':
            fprintf(stderr, "Syntaxe : %s [-a adresse] [-p port] \n", argv[0]);
            return -1;
        default:
            break;
        }
    }
    memset(adresse, 0, sizeof(struct sockaddr_in));

    // inet_aton() convertit une adresse IPv4 en notation pointée en une structure in_addr. Elle retourne 0 si l'adresse est invalide, sinon elle retourne un nombre non nul.
    if (inet_aton(hote, &(adresse->sin_addr)) == 0)
    {
        if ((hostent = gethostbyname(hote)) == NULL) // Convertis un nom d'hôte en une adresse IP. Elle retourne NULL si le nom d'hôte est inconnu.
        {
            fprintf(stderr, "hote = %s inconnu \n", hote);
            return -1;
        }
        adresse->sin_addr.s_addr = ((struct in_addr *)(hostent->h_addr))->s_addr;
    }

    if (sscanf(port, "%d", &numero) == 1) // sscanf() convertit une chaîne de caractères en un entier. Elle retourne le nombre d'éléments convertis et assignés. Si le port est un nombre, on le convertit en entier et on le met dans la structure sockaddr_in
    {
        adresse->sin_port = htons(numero);
        return 0;
    }

    if ((servent = getservbyname(port, protocole)) == NULL) // getservbyname() convertit un nom de service en un numéro de port.
    {
        fprintf(stderr, "Services %s inconnu \n", port);
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

    if (lecture_arguments(argc, argv, &adresse, "tcp") < 0) // Lecture des arguments de la ligne de commande. Si la lecture échoue, on quitte le programme
    {
        exit(EXIT_FAILURE);
    }
    adresse.sin_family = AF_INET;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // socket() permet de créer une socket. Elle retourne -1 si la création échoue, sinon elle retourne un descripteur de fichier pour la socket créée.
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&adresse, sizeof(struct sockaddr_in)) < 0) // connect() permet d'établir une connexion avec le serveur. Elle retourne -1 si la connexion échoue, sinon elle retourne 0.
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    // setvbuf(stdout, NULL, _IONBF, 0); // Permet d'agir sur un tampon en fonction du mode choix choisi. Ici, on choisit le mode _IONBF (pas de tampon) pour que les sorties soient affichées immédiatement.

    send(sock, "Hi\n", 3, 0); // Envoie un message "Hello" au client connecté à la socket

    while (1)
    {
        if ((nb_lus = read(sock, buffer, LG_BUFFER)) == 0) // Si le serveur a fermé la connexion, on sort de la boucle
        {
            break;
        }

        if (nb_lus < 0) // Si la lecture a échoué, on affiche un message d'erreur et on quitte le programme
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        write(STDOUT_FILENO, buffer, nb_lus); // Affiche le message reçu du serveur sur la sortie standard
    }

    return EXIT_SUCCESS;
}

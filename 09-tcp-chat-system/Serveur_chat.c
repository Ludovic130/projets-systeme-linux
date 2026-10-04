#include "lib.h"

#define NB_FILS 10

// int attente_reception(int descripteurs[], int nb_descripteurs, int delai_maxi)
// {
//     struct timeval attente;
//     fd_set ensemble;
//     int plus_grand = -1;
//     int i;
//     int retour;

//     attente.tv_sec = delai_maxi;
//     attente.tv_usec = 0;

//     /*initialisation de l'ensemble*/
//     FD_ZERO(&ensemble);

//     for (i = 0; i < nb_descripteurs; i++)
//     {
//         if (descripteurs[i] > FD_SETSIZE)
//         {
//             fprintf(stderr, "Descripteur trop grand");
//             return -1;
//         }

//         FD_SET(descripteurs[i], &ensemble);
//         if (descripteurs[i] > plus_grand)
//         {
//             plus_grand = descripteurs[i];
//         }
//     }

//     /* Attente */
//     do
//     {
//         /* code */
//         retour = select(plus_grand + 1, &ensemble, NULL, NULL, &attente);

//     } while ((retour == -1) && (errno = EINTR));

//     if (retour == 0)
//     {
//         fprintf(stderr, "délai dépasser\n");
//         return -1;
//     }

//     if (retour < 0)
//     {
//         perror("select");
//         exit(EXIT_FAILURE);
//     }

//     /* examens des descripteur prêt */
//     for (i = 0; i < nb_descripteurs; i++)
//     {
//         if (FD_ISSET(descripteurs[i], &ensemble))
//         {
//             lecture_descripteur(descripteurs[i]);
//         }
//     }

//     return 0;
// }

int main(void)
{
    int tubes[NB_FILS][2];
    fd_set ensemble;
    int i, fils;
    char c = 'c';

    for (i = 0; i < NB_FILS; i++)
    {
        if (pipe(tubes[i]) < 0) // pipe() permet de créer un tube anonyme. Elle retourne -1 si la création échoue, sinon elle retourne 0.
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (fils = 0; fils < NB_FILS; fils++)
    {
        if (fork() == 0) //
        {
            break;
        }
    }

    for (i = 0; i < NB_FILS; i++)
    {
        if (fils == NB_FILS) // On est dans le père
        {
            /* On est dans le père */
            close(tubes[i][1]);
        }
        else // fils (methodes pour savoir quel tube utiliser)
        {
            close(tubes[i][0]);
            if (i != fils)
            {
                close(tubes[i][1]);
            }
        }
    }

    if (fils == NB_FILS) // On est dans le père
    {
        while (1)
        {
            FD_ZERO(&ensemble); // On vide l'ensemble des descripteurs à surveiller

            for (i = 0; i < NB_FILS; i++)
            {
                FD_SET(tubes[i][0], &ensemble); // On ajoute le descripteur du tube à l'ensemble des descripteurs à surveiller
            }
            if (select(FD_SETSIZE, &ensemble, NULL, NULL, NULL) < 0) // select() permet de surveiller plusieurs descripteurs de fichiers. Elle retourne -1 si une erreur survient, sinon elle retourne le nombre de descripteurs prêts à être lus.
            {
                perror("select");
                break;
            }

            for (i = 0; i < NB_FILS; i++)
            {
                if (FD_ISSET(tubes[i][0], &ensemble)) // Verifie si le descripteur contient des donnees a lire. Si c'est le cas, on lit le descripteur et on affiche le numero du fils qui a envoyé le caractere
                {
                    fprintf(stdout, "%d ", i); // On affiche le numéro du fils qui a envoyé le caractère. Cela permet de voir l'ordre dans lequel les fils envoient des caractères au père.
                    fflush(stdout);            // On vide le tampon de sortie pour que l'affichage se fasse immédiatement
                    read(tubes[i][0], &c, 1);  // On lit le caractère envoyé par le fils. Cela permet de voir l'ordre dans lequel les fils envoient des caractères au père.
                }
            }
        }
    }
    else
    { /* On est dans le fils */
        while (1)
        {
            usleep((fils + 1) * 1000000); // Pour chaque fils, on attend une seconde de plus que le fils précédent avant d'envoyer un caractère au père. Cela permet de voir l'ordre dans lequel les fils envoient des caractères au père.
            write(tubes[fils][1], &c, 1); // On envoie un caractère au père. Le caractère est toujours 'c', mais on pourrait envoyer n'importe quel caractère. Cela permet de voir l'ordre dans lequel les fils envoient des caractères au père.
        }
    }
    return 0;
}

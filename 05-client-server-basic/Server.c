#include "lib.h"

static char *nom_server = "anagrammeLuc.fifo";

static int repondre(char const *my_fifoL, char *chaine)
{
    int fd;
    FILE *reponse;
    char *anagrammeR;

    if ((fd = open(my_fifoL, O_WRONLY)) >= 0) // Open the client's FIFO tube in write mode to send the response
    {
        if (fd < 0) //
        {
            if (errno == ENXIO) // If errno is equal to ENXIO then the server is not running
            {
                fprintf(stdout, "Le serveur n'est pas en cours d'exécution\n");
            }
            else
            {
                perror("open");
            }
            exit(EXIT_FAILURE);
        }
        reponse = fdopen(fd, "w");            // Convert the file descriptor into an output stream to write to the client's FIFO tube
        anagrammeR = strdup(chaine);          // Duplicate the string to process to create the anagram
        strfry(anagrammeR);                   // Randomly shuffle the characters of the string to create the anagram
        fprintf(reponse, "%s\n", anagrammeR); // Send the anagram to the client via the FIFO tube

        fclose(reponse);  // Close the output stream after sending the response to the client
        free(anagrammeR); // Free the memory allocated for the anagram after use
    }

    if ((strcasecmp(chaine, "FIN") == 0) || (strcasecmp(my_fifoL, "FIN") == 0)) // Compare the string to process with "FIN" to determine if the server should terminate
    {
        return 1;
    }

    return 0;
}

int main(void)
{
    FILE *fichier;
    int fd;
    char chaine[128];
    char my_fifoS[128];

    if (mkfifo(nom_server, 0644) != 0) // Create the server tube
    {
        fprintf(stdout, "Impossible de créer le fifo car un fichier existe deja");
        exit(EXIT_FAILURE);
    }

    if ((fd = open(nom_server, O_RDONLY)) < 0) // Open the server tube in read-only mode
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    fichier = fdopen(fd, "r"); // Convert the file descriptor into an input stream to read the client's response

    while (1)
    {
        if (fgets(my_fifoS, 128, fichier) == NULL) // If fgets returns NULL then close the stream and the server tube descriptor
        {
            fclose(fichier);
            close(fd);

            if ((fd = open(nom_server, O_RDONLY)) < 0) // Reopen the server tube
            {
                perror("Erropenserver");
                exit(EXIT_FAILURE);
            }

            fichier = fdopen(fd, "r"); // Convert the file descriptor into an input stream to read the client's response
            continue;                  // Return to the beginning of the loop to wait for a new client response
        }

        if (my_fifoS[strlen(my_fifoS) - 1] == '\n') // Remove the newline character '\n' at the end of the my_fifoS string
        {
            my_fifoS[strlen(my_fifoS) - 1] = '\0';
        }

        fgets(chaine, 128, fichier);

        if (chaine[strlen(chaine) - 1] == '\n') // Remove the newline character '\n' at the end of the chaine string
        {
            chaine[strlen(chaine) - 1] = '\0';
        }

        if (repondre(my_fifoS, chaine) != 0) // If the repondre function returns 1 then exit the loop and close the server tube
        {
            break;
        }
    }

    unlink(nom_server);
    return EXIT_SUCCESS;
}
#include "lib.h"

int main(void)
{
    FILE *question;
    FILE *reponse;
    int fd;
    char chaine[128];
    char my_fifoC[128];

    fprintf(stdout, "Répondre en écriture : ");
    if (fgets(chaine, 128, stdin) == NULL) // Input the string to send to the server
    {
        perror("fgetsin");
    }

    sprintf(my_fifoC, "anagramme.%ld", (long)getpid()); // Name of the FIFO tube for the server's response

    if (mkfifo(my_fifoC, 0644) != 0) // Create the FIFO tube for the server's response
    {
        fprintf(stdout, "Erreur de création du tube");
        exit(EXIT_FAILURE);
    }

    if ((fd = open("anagrammeLuc.fifo", O_WRONLY)) < 0) // Open the server's FIFO tube in write mode
    {
        fprintf(stdout, "Erreur d'ouverture du noeud du serveur");
        exit(0);
    }

    question = fdopen(fd, "w");                    // Convert the file descriptor into an output stream to write to the server's FIFO tube
    fprintf(question, "%s\n%s", my_fifoC, chaine); // Send the name of the FIFO tube for the response and the string to process to the server
    fclose(question);                              // Close the output stream after sending the question to the server

    fd = open(my_fifoC, O_RDONLY); // Open the FIFO tube for the server's response in read mode

    reponse = fdopen(fd, "r"); // Convert the file descriptor into an input stream to read the server's response

    if (fgets(chaine, 128, reponse) != NULL) // Read the server's response from the FIFO tube
    {
        fprintf(stdout, "reponse : %s", chaine);
    }
    else
    {
        perror("fgetsout");
    }

    fclose(reponse);  // Close the input stream after reading the server's response
    unlink(my_fifoC); // Remove the FIFO tube for the server's response after use

    return EXIT_SUCCESS;
}
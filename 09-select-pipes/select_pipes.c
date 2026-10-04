#include "lib.h"

#define NB_FILS 10

int main(void)
{
    int tubes[NB_FILS][2];
    fd_set ensemble;
    int i, fils;
    char c = 'c';

    for (i = 0; i < NB_FILS; i++)
    {
        if (pipe(tubes[i]) < 0) // pipe() creates an anonymous pipe. It returns -1 if creation fails, otherwise it returns 0.
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (fils = 0; fils < NB_FILS; fils++)
    {
        if (fork() == 0)
        {
            break;
        }
    }

    for (i = 0; i < NB_FILS; i++)
    {
        if (fils == NB_FILS) // We are in the parent
        {
            /* We are in the parent */
            close(tubes[i][1]);
        }
        else // child (methods to know which pipe to use)
        {
            close(tubes[i][0]);
            if (i != fils)
            {
                close(tubes[i][1]);
            }
        }
    }

    if (fils == NB_FILS) // We are in the parent
    {
        while (1)
        {
            FD_ZERO(&ensemble); // We clear the set of descriptors to monitor

            for (i = 0; i < NB_FILS; i++)
            {
                FD_SET(tubes[i][0], &ensemble); // We add the pipe descriptor to the set of descriptors to monitor
            }
            if (select(FD_SETSIZE, &ensemble, NULL, NULL, NULL) < 0) // select() monitors multiple file descriptors. It returns -1 on error, otherwise it returns the number of descriptors ready to be read.
            {
                perror("select");
                break;
            }

            for (i = 0; i < NB_FILS; i++)
            {
                if (FD_ISSET(tubes[i][0], &ensemble)) // Check if the descriptor has data to read. If so, we read the descriptor and print the number of the child that sent the character
                {
                    fprintf(stdout, "%d ", i); // We print the number of the child that sent the character. This lets us see the order in which the children send characters to the parent.
                    fflush(stdout);            // We flush the output buffer so that the display happens immediately
                    read(tubes[i][0], &c, 1);  // We read the character sent by the child. This lets us see the order in which the children send characters to the parent.
                }
            }
        }
    }
    else
    { /* We are in the child */
        while (1)
        {
            usleep((fils + 1) * 1000000); // For each child, we wait one more second than the previous child before sending a character to the parent. This lets us see the order in which the children send characters to the parent.
            write(tubes[fils][1], &c, 1); // We send a character to the parent. The character is always 'c', but any character could be sent. This lets us see the order in which the children send characters to the parent.
        }
    }
    return 0;
}
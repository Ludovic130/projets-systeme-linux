#include "lib.h"

#define NB_THREADS 5

int table[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
int taille_tab = sizeof(table) / sizeof(table[0]); // 44 / 4 = 11

typedef struct threads
{
    int threads_id, debut, fin;
} t_;

void *tab(void *inters)
{
    t_ *inter = (t_ *)inters; // Convert the void pointer into a pointer to the structure type
    int somme = 0;            // Always initialize the variable, otherwise it returns an unknown value (negative or positive)

    printf("Thread %d processes indices from %d to %d: \n", inter->threads_id, inter->debut, inter->fin);

    for (int i = inter->debut; i <= inter->fin; i++)
    {
        // printf("%d \n", table[i]);
        somme += table[i];

        if (i == inter->fin) // the index equals the final index, so we print
        {
            printf("the sum of thread %d is %d \n", i, somme);
        }
    }
    printf("\n");

    pthread_exit(NULL); // terminate the calling thread
}

int main(void)
{
    pthread_t threads[NB_THREADS];
    t_ inter[NB_THREADS];
    int i;
    int ret;

    int nb_chacun = taille_tab / NB_THREADS; // Ex: 11 / 5 = 2
    int reste = taille_tab % NB_THREADS;     // Ex: 11 % 5 = 1
    int index_actuel = 0;                    // We initialize index_actuel to 0

    for (i = 0; i < NB_THREADS; i++)
    {
        int index_supple = 0;
        inter[i].threads_id = i;       // We assign the thread's index to the structure at each iteration
        inter[i].debut = index_actuel; // We assign index_actuel to the structure at each iteration

        // Check if there is a remaining index lower than the remainder of the division
        if (i < reste)
        {
            index_supple = 1;
        }
        else if (i == reste)
        {
            index_supple = 0;
        }

        // Define the limit of the current thread
        inter[i].fin = index_actuel + nb_chacun - 1 + index_supple;

        // Update index_actuel for the next thread
        index_actuel = inter[i].fin + 1;

        // Create a thread
        // FIX: the original code had a precedence bug; the parenthesis must close after the call to pthread_create
        if ((ret = pthread_create(&threads[i], NULL, tab, (void *)&inter[i])) != 0)
        {
            fprintf(stderr, "%s", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    // Suspend the calling thread (main) until each thread finishes
    for (i = 0; i < NB_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
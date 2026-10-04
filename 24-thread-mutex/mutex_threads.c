#include "lib.h"

#define NB_THREADS 2 // Two threads

static void *fn_thread(void *compteur); // Function that the thread will execute
static int aleatoire(int maximum);      // Function that returns a random number

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // We initialize the mutex statically; it is defined as a global variable

static int compteur = 0;

int main(void)
{
    pthread_t threads[NB_THREADS]; // Define the threads variable
    int i;                         // Variable used to increment
    int ret;                       // variable that will store the value returned by pthread_create

    for (i = 0; i < NB_THREADS; i++)
    {
        if ((ret = pthread_create(&threads[i], NULL, fn_thread, (void *)(intptr_t)i)) != 0)
        {
            fprintf(stderr, "%s", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < NB_THREADS; i++)
    {
        if ((pthread_join(threads[i], NULL)) != 0) // pthread_join retrieves the thread's return value and blocks the calling thread (main())
        {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}

void *fn_thread(void *num)
{
    int numero = (int)(intptr_t)num; // Convert the value into the argument of the function that the thread will execute

    while (compteur < 40)
    {
        sleep(aleatoire(3));        // the thread will sleep for a randomly chosen number of seconds
        pthread_mutex_lock(&mutex); // Lock the thread that holds the mutex
        fprintf(stdout, "Thread %d of Ludovic acquired the mutex\n", numero);
        compteur++; // +1 at each iteration
        fprintf(stdout, "Thread %d : compteur = %d \n", numero, compteur);
        sleep(aleatoire(3)); // sleep again
        fprintf(stdout, "Thread %d of Ludovic released the mutex \n", numero);
        pthread_mutex_unlock(&mutex); // release the mutex
    }
    pthread_exit(NULL);
}

static int aleatoire(int maximum)
{
    double d;
    d = (double)maximum * rand(); // multiply the maximum by a random number
    d = d / (RAND_MAX + 1.0);
    return ((int)d);
}
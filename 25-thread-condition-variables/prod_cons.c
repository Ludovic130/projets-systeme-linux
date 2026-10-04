#include "lib.h"

#define SIZE 5

pthread_cond_t condition_buffer = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;

static void *thread_push(void *arg);
static void *thread_pop(void *arg);
static int aleatoire(int maximum);

static int BUFFER[SIZE];
static int head_buffer = 0;
static int tail_buffer = 0;
static int count = 0;
int val = 0;
static int running = 1;

int main(void)
{
    pthread_t threads;

    pthread_create(&threads, NULL, thread_push, NULL);
    pthread_create(&threads, NULL, thread_pop, NULL);

    pthread_exit(NULL);
}

static void *thread_push(void *arg)
{
    while (running)
    {
        pthread_mutex_lock(&mutex_buffer); // Lock the mutex

        if (count == SIZE) // count is
        {
            pthread_cond_wait(&condition_buffer, &mutex_buffer); // Wait for the thread to be woken up
        }

        sleep(aleatoire(2));

        if (val < 5)
        {
            val++;
            printf("--> %d\n", val);
            printf("thread 1 added %d to the buffer\n", val);
            BUFFER[head_buffer] = val;
            head_buffer = (head_buffer + 1) % SIZE; // Compute the remainder to find empty slots
            count++;
        }

        pthread_cond_signal(&condition_buffer);
        pthread_mutex_unlock(&mutex_buffer);
    }

    pthread_exit(NULL);
}

static void *thread_pop(void *arg)
{
    while (running)
    {
        pthread_mutex_lock(&mutex_buffer); // 1. Lock the mutex by the thread
        if (count == 0)
        {
            pthread_cond_wait(&condition_buffer, &mutex_buffer); // Wait for the thread to be woken up
        }

        if (val == SIZE)
        {
            count = BUFFER[tail_buffer];
            tail_buffer = (tail_buffer + 1) % SIZE;
            count--;
            printf("Thread removed %d from the buffer\n", val);
            val = 0;
        }

        pthread_cond_signal(&condition_buffer);
        pthread_mutex_unlock(&mutex_buffer); // release the thread after the condition is met
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
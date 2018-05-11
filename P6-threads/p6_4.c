#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *thrdfunc(void *tid)
{
    sleep(1);
    printf("Hello, I'm thread #%d\n", * (int *) tid);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

    int N = 5;
    pthread_t threads[N];
    int threadn[N];

    for (int i = 1; i <= N; i++)
    {
        threadn[i] = i;
        pthread_create(&threads[i], NULL, thrdfunc, &threadn[i]);
    }

    pthread_exit(0);
}
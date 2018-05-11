#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define STDERR 2
#define NUMITER 10000

void *thrfunc(void *arg)
{
    fprintf(stderr, "Starting thread %d\n", * (int *) arg);
    
    int i;
    write(STDOUT_FILENO, (int *) &arg, sizeof(arg));
    return NULL;
}
int main()
{
    pthread_t ta, tb;

    int n1 = 1;
    int n2 = 2;
    pthread_create(&ta, NULL, thrfunc, (void *) &n1);
    pthread_create(&tb, NULL, thrfunc, (void *) &n2);

    pthread_join(ta, NULL);
    pthread_join(tb, NULL);

    return 0;
}
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define STDERR 2

int N = 50;

void *thrfunc(void *arg)
{
    int counter = 0;
    fprintf(stderr, "Starting thread %s\n", (char *)arg);
    while (N > 0)
    {
        write(STDERR, arg, 1);
        N--;
        counter++;
    }
    
    return (void *) counter;
}

int main()
{
    pthread_t ta, tb;
    void *status1, *status2; 

    pthread_create(&ta, NULL, thrfunc, "1");
    pthread_create(&tb, NULL, thrfunc, "2");

    pthread_join(ta, &status1);
    pthread_join(tb, &status2);

    printf("\n%d\n",(int)status1);   
    printf("\n%d\n",(int)status2);   

    return 0;
}
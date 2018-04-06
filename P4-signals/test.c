#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(void) {

    pid_t pid;
    int status;

    pid = fork();

    if (pid > 0)
    {
        wait(&status);
        sleep(3);
        printf("Im dad");
    }
    else 
    {
        sleep(1);
        printf("Im child");
    }
}
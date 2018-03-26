#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int status;

int main(void)
{

    if (fork() > 0) {
        wait(NULL); //waits for any child process to finish
        //wait(&status); //waits for child termination w/ info about how it terminated
        printf("world!");
    }
    else {
        printf("Hello, ");
    }

    return 0;
}
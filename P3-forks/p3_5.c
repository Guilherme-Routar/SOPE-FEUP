#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int status;

int main(void)
{

    if (fork() > 0) { //grandpa
        wait(&status);
        printf("friends!");
    }
    else {
        if (fork() > 0 ) { //father
            wait(&status);
            printf("my ");
        }
        else { //son
            printf("Hello ");
        }
    }

    return 0;
}
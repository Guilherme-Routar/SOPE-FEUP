#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int var = 0;
int incFlag = 1;

void sigint_handler(int signo) {
    if (signo == SIGUSR1)
        incFlag = 1;
    if (signo == SIGUSR2)
        incFlag = 0;
}

int main(void) {

    // a)
    /*
    struct sigaction action;
    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    while(1) {
        printf("%d\n", var);
        if (incFlag == 1)
            var++;
        else var--;
        sleep(1);
    }
    */

    exit(0);
}
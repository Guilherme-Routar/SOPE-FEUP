#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

int var = 0;
int incFlag = 1;

void sigint_handler(int signo) {
    if (signo == SIGUSR1) {
        incFlag = 1;
        printf("SIGUSR1 triggered - Increment var\n");
    }
    if (signo == SIGUSR2) {
        incFlag = 0;
        printf("SIGUSR2 triggered - Decrement var\n");
    }
}

int main(void) {

    int status;
    pid_t pid;
    pid = fork();

    if (pid > 0) {
        printf("Dad launched with pid %d\n", getpid());
        
        int signal = 1, sleepTime;
        for (int i = 0; i < 10; i++) {
            sleepTime = rand() % 10;
            sleep(sleepTime);
            if (signal == 1) {
                kill(pid, SIGUSR1);
                signal = 2;
            }
            else {
                kill(pid, SIGUSR2);
                signal = 1;
            }
        }
        wait(&status);
    }
    else if (pid == 0) {
        printf("Child launched with pid %d\n", getpid());

        struct sigaction action;
        action.sa_handler = sigint_handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        if (sigaction(SIGUSR1, &action, NULL) < 0)
            perror("Failed to execute SIGUSR1 handler");
        if (sigaction(SIGUSR2, &action, NULL) < 0)
            perror("Failed to execute SIGUSR2 handler");
        
        for (int i = 0; i < 50; i++) {
            if (incFlag == 1) var++;
            else var--;
            printf("Var = %d\n", var);
            sleep(1);
        }
        exit(0);
    }
    else {
        perror("Fork failed");
    }

    exit(0);
}
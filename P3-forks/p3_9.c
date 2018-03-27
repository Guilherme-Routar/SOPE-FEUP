#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
    pid_t pid;
    int status;

    if (argc != 2)
    {
        printf("usage: %s dirname\n", argv[0]);
        exit(1);
    }
    pid = fork();
    if (pid > 0) {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) printf("Terminated successfully");
        if (WIFSIGNALED(status)) printf("Terminated abnormally");
    }
    else if (pid == 0)
    {
        execlp("ls", "ls -laR", argv[1], NULL);

        printf("Command not executed !\n");
        exit(1);
    }
    exit(0);
}
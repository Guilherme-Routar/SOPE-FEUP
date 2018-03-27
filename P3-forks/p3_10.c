#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
    pid_t pid;

    if (argc < 2)
    {
        printf("usage: %s dirname (outputfile)\n", argv[0]);
        exit(1);
    }

    pid = fork();
    if (pid == 0)
    {
        if (argc == 3)
        {
            int fd = open(argv[2], 0666);
            dup2(fd, STDOUT_FILENO);
        }

        execlp("ls", "ls -laR", argv[1], NULL);

        printf("Command not executed !\n");
        exit(1);
    }
    exit(0);
}
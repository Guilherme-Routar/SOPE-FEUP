#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define READ 0
#define WRITE 1

int main(int argc, char *argv[])
{

    // Reading file content
    char *filename = argv[1];
    char buff[255];
    char string[100000];

    FILE *filetoread = fopen(filename, "r");
    while (fgets(buff, 255, filetoread) != NULL)
    {
        strcat(string, buff);
    }

    // Sending file content through pipe
    int fd[2];
    pipe(fd);

    pid_t pid;
    pid = fork();

    if (pid > 0) // dad
    {
        close(fd[READ]);
        write(fd[WRITE], string, 100000 * sizeof(char));
        close(fd[WRITE]);
    }
    else // son
    { 
        char string[100000];
        close(fd[WRITE]);
        read(fd[READ], string, 100000 * sizeof(char));
        printf("%s", string);
        close(fd[READ]);
    }

    return 0;
}
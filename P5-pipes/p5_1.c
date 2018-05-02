#include <stdio.h>
#include <unistd.h>

#define READ 0
#define WRITE 1

struct numbers
{
    int n1;
    int n2;
};

int main(int argc, char *argv[])
{

    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();

    if (pid > 0) //dad
    {
        struct numbers n;
        scanf("%d %d", &n.n1, &n.n2);
        close(fd[READ]);
        write(fd[WRITE], &n, 2 * sizeof(int));
        close(fd[WRITE]);
    }
    else if (pid == 0) //child
    {
        struct numbers n;
        close(fd[WRITE]);
        read(fd[READ], &n, 2 * sizeof(int));
        printf("%d", n.n1 + n.n2);
        close(fd[READ]);
    }

    return 0;
}
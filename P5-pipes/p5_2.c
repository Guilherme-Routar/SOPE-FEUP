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
    int fd2[2];

    pipe(fd);
    pid = fork();

    if (pid > 0) //dad
    {
        struct numbers n;
        printf("x y ? ");
        scanf("%d %d", &n.n1, &n.n2);
        close(fd[READ]);
        write(fd[WRITE], &n, 2 * sizeof(int));
        close(fd[WRITE]);

        int result;
        close(fd2[WRITE]);
        read(fd2[READ], &result, sizeof(int));
        printf("result = %d", result);
        close(fd2[READ]);
    }
    else if (pid == 0) //child
    {
        
        struct numbers n;
        close(fd[WRITE]);
        read(fd[READ], &n, 2 * sizeof(int));
        close(fd[READ]);

        int result = n.n1 + n.n2;
        close(fd2[READ]);
        write(fd2[WRITE], &result, sizeof(int));
        close(fd2[WRITE]);
    }

    return 0;
}
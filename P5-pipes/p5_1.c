#include <stdio.h>
#include <unistd.h>

#define READ 0
#define WRITE 1

int main(int argc, char * argv[]) {

    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();

    if (pid > 0) //dad
    {
        int n[2];
        scanf("%d %d", &n[0], &n[1]);
        close(fd[READ]);
        write(fd[WRITE], n, 2*sizeof(int));
        close(fd[WRITE]);
    }
    else if (pid == 0) //child
    {
        close(fd[WRITE]);
        int n[2];
        read(fd[READ], n, 2*sizeof(int));
        printf("%d", n[0] + n[1]);
        close(fd[READ]);
    }

    return 0;
}
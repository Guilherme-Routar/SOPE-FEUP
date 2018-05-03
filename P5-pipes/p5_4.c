#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define READ 0
#define WRITE 1

int main(int argc, char *argv[])
{

    // ls <dir> -laR | grep <arg> | sort

    int status;

    int fd_ls_grep[2];
    int fd_grep_sort[2];
    pipe(fd_ls_grep);
    pipe(fd_grep_sort);

    pid_t pid;
    pid = fork();

    if (pid > 0)
    {
        // Redirects ls' stdout to pipe's writing end
        dup2(fd_ls_grep[WRITE], STDOUT_FILENO);

        close(fd_ls_grep[WRITE]);
        close(fd_ls_grep[READ]);
        close(fd_grep_sort[WRITE]);
        close(fd_grep_sort[READ]);

        execlp("ls", "ls", argv[1], "-laR", NULL);
    }
    else if (pid == 0)
    {
        pid_t pid2;
        pid2 = fork();

        if (pid2 > 0)
        {
            // Redirects grep's stdin to pipe's reading end
            // Redirects grep's stdout to another pipe's writing end
            dup2(fd_ls_grep[READ], STDIN_FILENO);
            dup2(fd_grep_sort[WRITE], STDOUT_FILENO);

            close(fd_ls_grep[WRITE]);
            close(fd_ls_grep[READ]);
            close(fd_grep_sort[WRITE]);
            close(fd_grep_sort[READ]);

            execlp("grep", "grep", argv[2], NULL);
        }
        
        else if (pid2 == 0)
        {
            dup2(fd_grep_sort[READ], STDIN_FILENO);

            close(fd_ls_grep[WRITE]);
            close(fd_ls_grep[READ]);
            close(fd_grep_sort[WRITE]);
            close(fd_grep_sort[READ]);

            execlp("sort", "sort", NULL);
        }
    }

    for (int i = 0; i < 2; i++)
        wait(&status);
}
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc, char *argv[], char *envp[])
{

    /**
     * l : arguments are passed as a list of strings to the main()
     * v : arguments are passed as an array of strings to the main()
     * p : path/s to search for the new running program
     * e : the environment can be specified by the caller
     * 
     * int execl(const char *path, const char *arg, ...);
     * int execlp(const char *file, const char *arg, ...);
     * int execle(const char *path, const char *arg, ..., char * const envp[]);
     * int execv(const char *path, char *const argv[]);
     * int execvp(const char *file, char *const argv[]);
     * int execvpe(const char *file, char *const argv[], char *const envp[])
    **/

    pid_t pid;
    if (argc != 2)
    {
        printf("usage: %s dirname\n", argv[0]);
        exit(1);
    }
    pid = fork();
    if (pid > 0)
        printf("My child is going to execute command \"ls -laR %s\"\n", argv[1]);
    else if (pid == 0)
    {
        // a)
        //execlp("ls", "ls -laR", argv[1], NULL); //argv[1] is the dir name to ls

        // b)
        //execl("/bin/ls", "ls -laR", argv[1], NULL);

        // c)
        char* array[] = {"ls", "-laR", argv[1], NULL};
        //execvp("ls", array);

        // d)
        //execv("/bin/ls", array);

        // e)
        execve("/bin/ls", array, envp);

        printf("Command not executed !\n");
        exit(1);
    }
    exit(0);
}
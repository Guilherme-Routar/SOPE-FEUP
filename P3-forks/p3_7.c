#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

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
    
    char prog[20];
    sprintf(prog, "%s.c", argv[1]);
    
    // Compiles the program/file with given by the user when running this executable
    execlp("gcc", "gcc", prog, "-Wall", "-o", argv[1], NULL);
    
    // After exec, this code is ignored
    printf('Done with this');
    exit(0);
}
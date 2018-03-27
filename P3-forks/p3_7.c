#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    
    char prog[20];
    sprintf(prog, "%s.c", argv[1]);
    
    // Compiles the program/file with given by the user when running this executable
    execlp("gcc", "gcc", prog, "-Wall", "-o", argv[1], NULL);
    
    // After exec, this code is ignored
    printf('Exec failed');
    exit(1);
}
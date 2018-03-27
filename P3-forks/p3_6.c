#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
    /**
     * A ZOMBIE processs is a process that has completed execution (via the exit system call)
     * but stil has an entry in the process table, even though its resource and memory
     * have been released. This occurs for child processes where the entry is still 
     * needed to allow the parent process to read its child's exit status
     * 
     * The zombie process is reaped (oficially dies) if the parent dies
     * 
     * If the parent dies and the child processes are still alive, they become orphan
     * and get adopted by init (pid=1)
    **/

    /**
     * Forking twice avoids the creation of zombie processes without defeating the 
     * purpose of fork's parallelism
     * 
     * Process A first forks a child process (B) and B then forks its child process (C).
     * Process B terminates as soon as process C is created. This way, A only has to
     * wait for B for a short period. Since C has no parent (B died), the system will adopt
     * C to the init process. Init calls wait() for its child process, avoiding the zombie
     **/

    pid_t pid1, pid2;
    int i, j, status;

    pid1 = fork(); // Process A creates B
    if (pid1 > 0) {
        waitpid(pid1, &status, NULL); // A waits for B
        for (j = 1; j <= 10; j++) {
            sleep(1);
            printf("father working ...\n");
        }
    }
    else if (pid1 == 0) {
        pid2 = fork(); // Process B creates C
        if (pid2 > 0) {
            exit(0); // B terminates as soon as C is created
        }
        else if (pid2 == 0) { // C does whatever A wanted B to do
            printf("I'm process %d. My parent is %d. I'm going to work for 1 second ...\n", getpid(), getppid());
            sleep(3); // simulando o trabalho do filho
            printf("I'm process %d. My parent is %d. I finished my work\n", getpid(), getppid());
            exit(0); // a eliminar na alinea c)
        }
        else {
            /* error */
        }
    }
    else {
        /* error */
    }

    exit(0);
}
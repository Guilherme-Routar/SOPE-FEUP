#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

/**
* CTRL+C should output an exit prompt. Continue if N, exit if Y 
**/
void sigint_handler(int signo) 
{
    printf("Are you sure you want to quit? (Y/N): ");
    char str[1];
    scanf("%s", str);
    str[0] = toupper(str[0]);
    if (strcmp(str, "Y") == 0) {
        printf("Quitting\n");
        exit(0);
    }
    if (strcmp(str, "N") == 0) printf("Continuing\n");
}

void check_CTRL_C() 
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

int main(int argc, char *argv[])
{

    check_CTRL_C();

    // User must insert at least a pattern and a file (or directory)
    if (argc < 3) {
        perror("Insuficient Arguments\n");
        return 0;
    }

    /**
     * Read from STDIN in case there is no cmd line argument for the file or directory
    **/
    if (!feof(stdin))
        printf("stdin is empty\n");

    /**
     * [OPTIONS]
     * -i : ignore letters size (upper, lower)
     * -l : display only the names of the files where the pattern is being searched 
     * -n : indicate the number of the lines where the pattern should be searched
     * -c : indicate how many lines it took to find the pattern
     * -w : the pattern should be a full word
     * -r : search the pattern in every file below the indicated directory
    **/

   sleep(10);

printf("After sleep");

sleep(3);
    
}
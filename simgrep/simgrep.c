#include <stdio.h>
#include <signal.h> //signals
#include <string.h>
#include <stdlib.h> //venv
#include <dirent.h> //getDirContent

#define CURRENT ".";

/**
* CTRL+C should output an exit prompt. Continue if N, exit if Y 
**/
void sigint_handler(int signo)
{
    printf("Are you sure you want to quit? (Y/N): ");
    char str[1];
    scanf("%s", str);
    str[0] = toupper(str[0]);
    if (strcmp(str, "Y") == 0)
    {
        printf("Quitting\n");
        exit(0);
    }
    if (strcmp(str, "N") == 0)
        printf("Continuing\n");
}

void check_CTRL_C()
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGINT handler\n");
    }
}

char * getDirContent(char *directory)
{
    char * content[100];

    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d)
    {
        int index = 0;
        while ((dir = readdir(d)) != NULL)
        {
            char name[255];
            strncpy(name, dir->d_name, 100);
            content[index] = name;
            printf("\nstring = %s", content[index]);
            index++;
        }
        closedir(d);
    }
    return content;
}

int main(int argc, char *argv[])
{

    check_CTRL_C();

    // User must insert at least a pattern and a file (or directory)
    if (argc < 3)
    {
        fprintf(stderr, "Insuficient arguments\n");
        return 0;
    }

    /**
     * Read from STDIN in case there is no cmd line argument for the file or directory
    **/
    //if (!feof(stdin))
    //printf("stdin is empty\n");

    /**
     * [OPTIONS]
     * -i : ignore letters size (upper, lower)
     * -l : display only the names of the files where the pattern is being searched 
     * -n : indicate the number of the lines where the pattern should be searched
     * -c : indicate how many lines it took to find the pattern
     * -w : the pattern should be a full word
     * -r : search the pattern in every file below the indicated directory
    **/

    getDirContent("testgrep");

    //char * dir[] = {"string1", "string2"};
    //printf("%s", dir[0]);

    //printf("leaving simgrep");
    sleep(3);
    return 0;
}
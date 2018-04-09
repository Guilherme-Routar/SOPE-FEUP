#include <stdio.h>
#include <signal.h> //signals
#include <string.h>
#include <stdlib.h> //venv
#include <dirent.h> //getDirContent
#include <malloc.h>

#define CURRENT_FOLDER ".";

#define MAX_INSTANCES_PER_FOLDER 100
#define MAX_INSTANCE_NAME 80

#define LINE_MAX_LENGTH 255

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
    if (sigaction(SIGINT, &sa, NULL) < 0)
        fprintf(stderr, "Unable to install SIGINT handler\n");
}

char **initFolderArray()
{
    int i = 0;
    char **folderContent = malloc(sizeof(char *) * MAX_INSTANCES_PER_FOLDER);
    if (!folderContent)
        return NULL;
    for (i = 0; i < MAX_INSTANCES_PER_FOLDER; i++)
    {
        folderContent[i] = malloc(MAX_INSTANCE_NAME + 1);
        if (!folderContent[i])
        {
            free(folderContent);
            return NULL;
        }
    }
    return folderContent;
}

char **getFolderContent(char *directory)
{
    char **folderContent = initFolderArray();

    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d)
    {
        int index = 0;
        while ((dir = readdir(d)) != NULL)
        {
            strncpy(folderContent[index], (char *)dir->d_name, MAX_INSTANCE_NAME);
            index++;
        }
        closedir(d);
    }
    return folderContent;
}

/**
* Read from STDIN in case there is no cmd line argument for the file or directory
**/
void checkSTDIN()
{
    if (!feof(stdin))
        fprintf(stderr, "stdin is empty\n");
}

/**
 * Search substring pattern in file
**/
void searchPattern(char *pathToFile, char *pattern)
{
    FILE *fp;
    fp = fopen(pathToFile, "r");
    char line[LINE_MAX_LENGTH];
    while (fgets(line, 100, fp))
    {
        if (strstr(line, pattern) != NULL)
            printf("match in line: %s", line);
    }
}

/**
 * Search whole pattern in file
 * https://stackoverflow.com/questions/42352846/matching-an-exact-word-using-in-c
**/
void searchWholePattern(char *pathToFile, char *pattern)
{
    FILE *fp;
    fp = fopen(pathToFile, "r");
    char line[LINE_MAX_LENGTH];
    while (fgets(line, LINE_MAX_LENGTH, fp))
    {
        const char *p = line;
        for (;;)
        {
            p = strstr(p, pattern);
            if (p == NULL) break;

            if ((p == line) || !isalnum((unsigned char)p[-1]))
            {
                p += strlen(pattern);
                if (!isalnum((unsigned char)*p))
                {
                    printf("Match in line : %s\n", line);
                    break; // found, quit
                }
            }
            // substring was found, but no word match, move by 1 char and retry
            p += 1;
        }
    }
}

int main(int argc, char *argv[])
{

    check_CTRL_C();

    checkSTDIN();

    // User must insert at least a pattern and a file (or directory)
    if (argc < 3)
    {
        fprintf(stderr, "Insuficient arguments\n");
        return 0;
    }
    else if (argc == 3)
    {
        //no options
    }
    else
    {
        //handle options
    }

    /**
     * [OPTIONS]
     * 
     * -i : ignore letters size (upper, lower)
     * -l : display only the names of the files where the pattern is being searched 
     * -n : indicate the number of the lines where the pattern is being searched
     * -c : indicate how many lines it took to find the pattern
     * 
     * -w : the pattern should be a full word
     * -r : search the pattern in every file below the indicated directory
    **/

    char **content = getFolderContent("testgrep");

    char *pattern = "std";
    //searchPattern("simgrep.c", pattern);
    //searchWholePattern("simgrep.c", pattern);

    //printf("leaving simgrep");
    sleep(2);
    return 0;
}
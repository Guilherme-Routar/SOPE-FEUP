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
 * [OPTIONS]
**/
int IGNORE_CASE = 0;  // -i : Ignore letters case
int FILE_NAME = 0;    // -l : Display the names of the files where the pattern is
int LINE_NUMBER = 0;  // -n : Display the number of the lines where the pattern is
int LINE_COUNTER = 0; // -c : Display how many lines match the pattern
int WHOLE_WORD = 0;   // -w : The pattern is a whole word
int TREE = 0;         // -r : Breadth-first search

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

/**
 * Initializes a 2D array with the defined macros
**/
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

/**
 * Get content from the given directory
 * Returns an array of strings, containing files and/or folders
**/
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
* [OPTIONS]
* -i : ignore letters size (upper, lower)
* -l : display only the names of the files where the pattern is being searched 
* -n : indicate the number of the lines where the pattern matched
* -c : indicate how many lines it took to find the pattern
* -w : the pattern should be a full word //DONE
* -r : search the pattern in every file below the indicated directory
**/

/**
 * Search substring pattern in file
**/
void searchPattern(char *pathToFile, char *pattern, char *options)
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
void searchWholePattern(char *pathToFile, char *pattern, char *options)
{
    printf("Searching whole pattern..\n");

    FILE *fp;
    fp = fopen(pathToFile, "r");
    char line[LINE_MAX_LENGTH];
    while (fgets(line, LINE_MAX_LENGTH, fp))
    {
        const char *p = line;
        for (;;)
        {
            p = strstr(p, pattern);
            if (p == NULL)
                break;

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

void parseOptions(int argc, char *argv[])
{
    for (int i = 1; i < argc - 2; i++)
    {
        if (strcmp(argv[i], "-i") == 0) 
            IGNORE_CASE = 1;
        if (strcmp(argv[i], "-l") == 0) 
            FILE_NAME = 1;     
        if (strcmp(argv[i], "-n") == 0) 
            LINE_NUMBER = 1; 
        if (strcmp(argv[i], "-c") == 0) 
            LINE_COUNTER = 1; 
        if (strcmp(argv[i], "-w") == 0) 
            WHOLE_WORD = 1;
        if (strcmp(argv[i], "-r") == 0) 
            TREE = 1;  
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

    //char **content = getFolderContent("testgrep");

    char *file = "test.txt";
    char *pattern = argv[argc - 2];
    parseOptions(argc, argv);

    //printf("%d", IGNORE_CASE);

    /*
    if (strstr(options, 'w') != NULL)
        searchWholePattern(file, pattern, options);
    else
        searchPattern(file, pattern, options);
    */

    sleep(2);
    return 0;
}
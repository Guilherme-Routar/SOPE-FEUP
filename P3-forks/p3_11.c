#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[])
{
    char command[100];
    const char *dupCommand = command;
    char *token, *str, *tofree;

    char **strings = (char **)malloc(10 * sizeof(char *));
    for (int i = 0; i < 10; i++)
    {
        printf("%d\n", i);
        strings[i] = (char *)malloc(50 * sizeof(char));
    }

    while (1)
    {

        printf("minish > ");

        fgets(command, sizeof(command), stdin); //command stores the EOL char (\n)

        // Exit when quit is inserted
        if (strcmp(command, "quit\n") == 0)
            break;

        printf(command[0]);

        // Splitting each command
        tofree = str = strdup(dupCommand);
        int i = 0;
        while ((token = strsep(&str, " ")))
        {
            sprintf(strings[i], token);
            i++;
        }
        free(tofree);
        printf(strings[0]);
        for (i = 0; i < 10; i++)
        {
            free(strings[i]);
        }
        free(strings);
    }

    exit(0);
}
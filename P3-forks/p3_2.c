#include <stdio.h>
#include <unistd.h>

int main(void)
{

    // ### ex2 ### 
    // a
    
    write(STDOUT_FILENO, "1", 1);
    if (fork() > 0)
    {
        write(STDOUT_FILENO, "2", 1);
        write(STDOUT_FILENO, "3", 1);
    }
    else
    {
        write(STDOUT_FILENO, "4", 1);
        write(STDOUT_FILENO, "5", 1);
    }
    write(STDOUT_FILENO, "\n", 1);
    
    // b
    /*
    printf("1");
    if (fork() > 0)
    {
        printf("2");
        printf("3");
    }
    else
    {
        printf("4");
        printf("5");
    }
    printf("\n");

    return 0;
    */

    /*
        a) output: 123 \n 45
        b) output: 123 \n 145

        The write call (system call) is unbuffered, the input is immediatelly printed,
        while printf saves the input to the buffer, only printing the content in the 
        end of the execution

        In a), "1" is immediatly printed. In b), "1" is saved to both processes' buffer
        and is only printed in the end of the execution that's why it appears twice
    */
}
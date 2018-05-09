#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/**
 * Posts the numbers to fifo fifo_req
 * Reads the result from fifo_ans
**/

int readline(int fd, char *str);

int main(int argc, char *argv[])
{
    int fd;
    mkfifo("/tmp/fifo_chg", 0660);

    /*
    Sending values to the server
    */
    char str[20];
    sprintf(str, "Chegou %s", argv[1]);
    do
    {
        fd = open("/tmp/fifo_chg", O_WRONLY);
        if (fd == -1)
            sleep(1);
    } while (fd == -1);
    write(fd, str, sizeof(str));

    close(fd);
}
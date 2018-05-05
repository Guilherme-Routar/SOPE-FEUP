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
    int fd_req, fd_ans;
    mkfifo("/tmp/fifo_req", 0660);
    mkfifo("/tmp/fifo_ans", 0660);

    /*
    Sending values to the server
    */
    int n[2];
    printf("x y ? ");
    scanf("%d %d", &n[0], &n[1]);

    do
    {
        fd_req = open("/tmp/fifo_req", O_WRONLY);
        if (fd_req == -1)
            sleep(1);
    } while (fd_req == -1);
    write(fd_req, n, 2 * sizeof(int));

    close(fd_req);

    /*
    Printing result received from the server
    */
    int result;
    fd_ans = open("/tmp/fifo_ans", O_RDONLY);
    read(fd_req, &result, sizeof(int));

    printf("result = %d", result);

    close(fd_ans);
}

int readline(int fd, char *str)
{
    int n;
    do
    {
        n = read(fd, str, 1);
    } while (n > 0 && *str++ != '\0');
    return (n > 0);
}
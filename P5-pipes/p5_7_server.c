#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * Reads the numbers from fifo fifo_req
 * Posts the (sum) result to fifo_ans
**/

int readline(int fd, char *str);



int main(int argc, char *argv[])
{
    int fd_req, fd_ans;
    mkfifo("/tmp/fifo_req", 0660);
    mkfifo("/tmp/fifo_ans", 0660);

    /*
    Reading input from the client
    */
    int n[2];
    fd_req = open("/tmp/fifo_req", O_RDONLY);
    read(fd_req, n, 2 * sizeof(int));

    close(fd_req);

    /*
    Sending result back to client
    */
    do
    {
        fd_ans = open("/tmp/fifo_ans", O_WRONLY);
        if (fd_ans == -1)
            sleep(1);
    } while (fd_ans == -1);
    int result = n[0] + n[1];
    write(fd_ans, &result, sizeof(int));

    close(fd_ans);

    return 0;
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
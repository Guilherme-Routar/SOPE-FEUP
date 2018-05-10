#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

    int fd;
    do
    {
        fd = open("requests", O_WRONLY);
        if (fd == -1)
            sleep(1);
    } while (fd == -1);

    // Sending struct req to fifo request
    write(fd, &req, 101 * sizeof(int));

    close(fd);
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

    // Attempting to open the fifo request
    int fdreq;
    do
    {
        fdreq = open("testfifo", O_WRONLY);
        if (fdreq == -1)
            sleep(1);
    } while (fdreq == -1);

    // Sending struct req to fifo request
    //write(fdreq, "test", 4 * sizeof(char));

    close(fdreq);
}
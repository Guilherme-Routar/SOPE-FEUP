#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[])
{
    
    clock_t initial_time = clock();

    int fd;
    mkfifo("/tmp/fifo_chg", 0660);

    //reading input from client
    char str[20];
    fd = open("/tmp/fifo_chg", O_RDONLY | O_NONBLOCK);

    clock_t current_time = initial_time;
    while ((current_time - initial_time) < 500)
    {    
        current_time = clock();
        read(fd, str, sizeof(str));
        puts(str);
        
        sleep(1);
    }
    printf("Time elapsed");
    
    return 0;
}
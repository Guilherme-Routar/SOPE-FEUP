#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int readline(int fd, char *str);

int main(void)
{
    int fd;
    char str[100];
    mkfifo("/tmp/myfifo", 0660);
    fd = open("/tmp/myfifo", O_RDWR);
    while (readline(fd, str))
        printf("%s", str);
    close(fd);
    return 0;
}
int readline(int fd, char *str)
{
    int n;
    do
    {
        n = read(fd, str, 1); // Returns 0 when EOF if reader is O_RDONLY.
                              // If reader is O_RDWR read returns 1 (write still open)
        //printf("n = %d", n); // read's return value
    } while (n > 0 && *str++ != '\0');
    return (n > 0);
}
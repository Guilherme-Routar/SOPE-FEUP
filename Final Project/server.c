#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    clock_t initial_time = clock();

    sleep(5);

    clock_t time_after = clock();

    printf("elaped time = %d", time_after - initial_time);
}
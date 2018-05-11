#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "request.h"

void create_fifo_requests();
void get_client_requests();

int main(int argc, char *argv[])
{
  /* Checking server command line arguments */
  if (argc != 4)
  {
    fprintf(stderr, "Usage: server <num_room_seats> <num_ticket_offices> <open_time>");
    exit(0);
  }

  /* Creating fifo requests */
  create_fifo_requests();

  /* Waiting for client requests */
  char *end;
  int open_time = strtol(argv[3], &end, 10);
  get_client_requests(open_time);
}

void create_fifo_requests()
{
  if (mkfifo("requests", 0660) < 0)
    if (errno == EEXIST)
      printf("FIFO requests already created\n");
    else
      printf("Can't create FIFO requests\n");
  else
    printf("FIFO requests sucessfully created\n");
}

void get_client_requests(int open_time)
{
  int fd_req;
  if ((fd_req = open("requests", O_RDONLY, O_NONBLOCK)) == -1)
  {
    fprintf(stderr, "Unable to open FIFO requests");
    return;
  }

  clock_t initial_time = clock();
  clock_t current_time = initial_time;
  while ((current_time - initial_time) < (open_time * 100))
  {
    current_time = clock();

    /*
    char str[100];
    int n = read(fd_req, str, 100);
    if (n > 0) printf("request from client %s", str);
    */
    struct request req;
    int n = read(fd_req, &req, sizeof(req));
    if (n > 0) printf("request from client %d\n", req.num_wanted_seats);
    
    
    sleep(1);
  }
  printf("Time elapsed.");
}

void launch_ticket_offices_threads(int num_ticket_offices)
{
}
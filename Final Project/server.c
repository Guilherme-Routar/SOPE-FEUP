#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "request.h"

void create_fifo_requests();
void get_client_requests(int open_time);
int validate_request(struct request req, int num_room_seats);

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

    struct request req;
    int n = read(fd_req, &req, sizeof(req));
    if (n > 0)
      //printf("%d\n", req.pref_seats_size);
      validate_request(req, MAX_ROOM_SEATS);

    sleep(1);
  }

  printf("Time elapsed.");
}

int validate_request(struct request req, int num_room_seats)
{
  /* Validating number of wanted seats */
  if (!(req.num_wanted_seats >= 1 &&
        req.num_wanted_seats <= MAX_CLI_SEATS))
    return OVERFLOW_NUM_WANTED_SEATS;

  /* Validating size of prefered seats list */
  if (!(req.pref_seats_size >= req.num_wanted_seats &&
        req.pref_seats_size <= MAX_CLI_SEATS))
    return INVALID_NUMBER_PREF_SEATS;

  /* Validating number of each prefered seat */
  for (int i = 0; i < req.pref_seats_size; i++)
  {
    printf("ar = %d\n", req.pref_seat_list[i]);
  }
  /*
  for (int i = 0; i < req.pref_seats_size; i++)
  {
    if (!(req.pref_seat_list[i] >= 1 &&
          req.pref_seat_list[i] <= num_room_seats))
      return INVALID_SEAT_NUMBER; 
  }*/

  return 1;
}

void launch_ticket_offices_threads(int num_ticket_offices)
{
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "request.h"
#include "seats.h"

void create_fifo_requests();
void get_client_requests(int open_time);
int validate_request(Request req, int num_room_seats);
void *safe_malloc(void *ptr, int nbytes);
void init_seats_list(int num_room_seats);

Seat *seats;

int main(int argc, char *argv[])
{
  /* Checking server command line arguments */
  if (argc != 4)
  {
    fprintf(stderr, "Usage: server <num_room_seats> <num_ticket_offices> <open_time>");
    exit(0);
  }

  /* Creating fifo requests */
  //create_fifo_requests();

  char *end;
  int num_room_seats = strtol(argv[1], &end, 10);
  init_seats_list(num_room_seats);

  /* Waiting for client requests */
  int open_time = strtol(argv[3], &end, 10);
  //get_client_requests(open_time);

  /* Launch ticket offices threads */
  int num_ticket_offices = strtol(argv[2], &end, 10);
  //launch_ticket_offices_threads(num_ticket_offices);
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

void *safe_malloc(void * ptr, int nbytes)
{
  ptr = malloc(nbytes);
  if (!ptr) {
    fprintf(stderr, "Failed to allocate %d bytes of memory\n", nbytes);
    return BAD_ALLOC;
  }
  return ptr;
}

void init_seats_list(int num_room_seats)
{
  int nbytes = num_room_seats * sizeof(Seat);
  seats = safe_malloc(seats, nbytes);

  Seat s;
  for (int i = 0; i < num_room_seats; i++) {
    s.number = i + 1;
    s.free = true;
    seats[i] = s;
  }
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

    Request req;
    int n = read(fd_req, &req, sizeof(req));
    if (n > 0)
      printf("%d", validate_request(req, MAX_ROOM_SEATS));

    sleep(1);
  }

  printf("Time elapsed.");
}

int validate_request(Request req, int num_room_seats)
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
    if (!(req.pref_seat_list[i] >= 1 &&
          req.pref_seat_list[i] <= num_room_seats))
      return INVALID_SEAT_NUMBER;
  }

  for (int i = 0; i < req.pref_seats_size; i++)
  {
    printf("n = %d\n", req.pref_seat_list[i]);
  }

  return 1;
}

/*

void launch_ticket_offices_threads(int num_ticket_offices)
{
  pthread_t threads[num_ticket_offices];
  for (int i = 0; i < num_ticket_offices; i++) {
    pthread_create(&threads[i], NULL, ticket_office_thrfn, NULL);
  }
}

void *ticket_office_thrfn(void *arg) {

}

int isSeatFree() {

}

void bookSeat() {

}

void freeSeat() {

}

*/
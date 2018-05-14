#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <ctype.h>

#include "request.h"
#include "seats.h"

void create_fifo_requests();
void get_client_requests(int open_time);
int validate_request(Request req);

void *safe_malloc(void *ptr, int nbytes);
Seat *init_seats_list(Seat *seats);

void launch_ticket_offices_threads(int num_ticket_offices);
void *ticket_office_handler(void *arg);

int isSeatFree(Seat *seats, int seatNum);
void bookSeat(Seat *seats, int seatNum, int clientId);
void freeSeat(Seat *seats, int seatNum);

// Global variables
int num_room_seats;
Request request;   // 1u buffer
sem_t empty, full; // global semaphores

int main(int argc, char *argv[])
{
  /* Checking server command line arguments */
  if (argc != 4)
  {
    fprintf(stderr, "Usage: server <num_room_seats> <num_ticket_offices> <open_time>");
    exit(0);
  }

  Seat *seats;

  /* Creating fifo requests */
  create_fifo_requests();

  char *end;
  num_room_seats = strtol(argv[1], &end, 10);
  seats = init_seats_list(seats);

  sem_init(&empty, SHARED, 1);
  sem_init(&full, SHARED, 0);

  /* Launch ticket offices threads */
  int num_ticket_offices = strtol(argv[2], &end, 10);
  launch_ticket_offices_threads(num_ticket_offices);

  /* Waiting for client requests */
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

void *safe_malloc(void *ptr, int nbytes)
{
  ptr = malloc(nbytes);
  if (!ptr)
  {
    fprintf(stderr, "Failed to allocate %d bytes of memory\n", nbytes);
    return BAD_ALLOC;
  }
  return ptr;
}

void get_client_requests(int open_time)
{
  int fd_req;
  if ((fd_req = open("requests", O_RDONLY | O_NONBLOCK)) == -1)
  {
    fprintf(stderr, "Unable to open FIFO requests");
    return;
  }

  time_t initial_time = time(NULL);
  time_t current_time = initial_time;
  while ((current_time - initial_time) < open_time)
  {
    Request req;
    int n = read(fd_req, &req, sizeof(req));
    if (n > 0)
    {
      sem_wait(&empty);
      request = req;
      sem_post(&full);
      printf("req = %d\n", req.pid);
    }

    usleep(100000); // 100 ms
    current_time = time(NULL);
  }

  printf("Ticket offices closed.\n");
  unlink("requests");
}

int validate_request(Request req)
{
  int num_wanted_seats = req.num_wanted_seats;
  int pref_seats_size = req.pref_seats_size;

  if (!isdigit(num_wanted_seats))
    return INVALID_PARAMETERS;

  /* Validating number of wanted seats */
  if (!(num_wanted_seats >= 1 &&
        num_wanted_seats <= MAX_CLI_SEATS))
    return OVERFLOW_NUM_WANTED_SEATS;

  /* Validating size of prefered seats list */
  if (!(pref_seats_size >= num_wanted_seats &&
        pref_seats_size <= MAX_CLI_SEATS))
    return INVALID_NUMBER_PREF_SEATS;

  /* Validating number of each prefered seat */
  for (int i = 0; i < req.pref_seats_size; i++)
  {
    if (!(isdigit(req.pref_seat_list[i]) &&
          req.pref_seat_list[i] >= 1 &&
          req.pref_seat_list[i] <= num_room_seats))
      return INVALID_SEAT_NUMBER;
  }

  return VALID_REQUEST;
}

void launch_ticket_offices_threads(int num_ticket_offices)
{
  pthread_t threads[num_ticket_offices];
  for (int i = 0; i < num_ticket_offices; i++)
  {
    pthread_create(&threads[i], NULL, ticket_office_handler, NULL);
  }
}

void *ticket_office_handler(void *arg)
{
  Request myreq;

  FILE *fslog;
  fslog = fopen(SLOG, "a");
  fprintf(fslog, "%d-OPEN\n", pthread_self());

  while (1)
  {
    sem_wait(&full);
    myreq = request;
    sem_post(&empty);

    //printf("return = %d\n", validate_request(myreq));
    int request_status = validate_request(myreq);

    switch (request_status)
    {
    case VALID_REQUEST:
      // handle request
      
      // UNAVAILABLE_SEATS
      //fprintf(fslog, "%d-%d-%d: list of seats - %s\n", pthread_self(), myreq.pid, myreq.num_wanted_seats, "NAV");
      // FULL_ROOM
      //fprintf(fslog, "%d-%d-%d: list of seats - %s\n", pthread_self(), myreq.pid, myreq.num_wanted_seats, "FUL");
      break;
    case INVALID_PARAMETERS:
      fprintf(fslog, "%d-%d-%d: list of seats - %s\n", pthread_self(), myreq.pid, myreq.num_wanted_seats, "ERR");
      break;
    case OVERFLOW_NUM_WANTED_SEATS:
      fprintf(fslog, "%d-%d-%d: list of seats - %s\n", pthread_self(), myreq.pid, myreq.num_wanted_seats, "MAX");
      break;
    case INVALID_NUMBER_PREF_SEATS:
      fprintf(fslog, "%d-%d-%d: list of seats - %s\n", pthread_self(), myreq.pid, myreq.num_wanted_seats, "NST");
      break;
    case INVALID_SEAT_NUMBER:
      fprintf(fslog, "%d-%d-%d: list of seats - %s\n", pthread_self(), myreq.pid, myreq.num_wanted_seats, "IID");
      break;
    
    }

    /*
    if (request_status == VALID_REQUEST)
    {
      // seats = arg ...
      int booked_seats = 0;
      for (int i = 0; i < myreq.pref_seats_size; i++)
      {
        if (isSeatFree(seats, myreq.pref_seat_list[i]) == SEAT_AVAILABLE)
        {
          bookSeat(seats, myreq.pref_seat_list[i], myreq.pid);
          booked_seats++;
        }
      }
      if (booked_seats == myreq.num_wanted_seats)
        printf("Reserved successfully all the seats");
      else
      {
        //freeseats
      }
    } */

    //handling
    printf("client pid = %d\n", myreq.pid);
    printf("tid = %d\n", pthread_self());
  }

  fprintf(fslog, "%d-CLOSED\n", pthread_self());
}

Seat *init_seats_list(Seat *seats)
{
  int nbytes = num_room_seats * sizeof(Seat);
  seats = safe_malloc(seats, nbytes);

  Seat s;
  for (int i = 0; i < num_room_seats; i++)
  {
    s.number = i + 1;
    s.available = SEAT_AVAILABLE;
    s.client_id = -1;
    seats[i] = s;
  }

  return seats;
}

// Testing if seat seatNum is free
int isSeatFree(Seat *seats, int seatNum)
{
  Seat s = seats[seatNum - 1];
  if (s.available == SEAT_AVAILABLE)
    return SEAT_AVAILABLE;
  return SEAT_UNAVAILABLE;
}

// Booking seat seatnum for client clientId
void bookSeat(Seat *seats, int seatNum, int clientId)
{
  Seat s = seats[seatNum - 1];
  s.available = SEAT_UNAVAILABLE;
  s.client_id = clientId;
  seats[seatNum - 1] = s;
}

// Freeing seat seatNum (in case the final reservation couldn't be done)
void freeSeat(Seat *seats, int seatNum)
{
  Seat s = seats[seatNum - 1];
  s.available = SEAT_AVAILABLE;
  s.client_id = -1;
  seats[seatNum - 1] = s;
}
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
Seat *init_seats_list();

void launch_ticket_offices_threads(int num_ticket_offices);
void *ticket_office_handler(void *arg);

void free_booked_seats(int booked_seats_list[], int num_wanted_seats);
int isSeatFree(Seat *seat, int seatNum);
void bookSeat(Seat *seat, int seatNum, int clientId);
void freeSeat(Seat *seat, int seatNum);

int * init_booked_seats_list(int num_wanted_seats);

// Global variables
int num_room_seats;
Request request;   // 1u buffer
sem_t empty, full; // global semaphores
Seat *seats;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

  char *end;
  num_room_seats = strtol(argv[1], &end, 10);
  init_seats_list();

  sem_init(&empty, SHARED, 1);
  sem_init(&full, SHARED, 0);

  /* Launch ticket offices threads */
  int num_ticket_offices = strtol(argv[2], &end, 10);
  launch_ticket_offices_threads(num_ticket_offices);

  /* Waiting for client requests */
  int open_time = strtol(argv[3], &end, 10);
  get_client_requests(open_time);

  sem_destroy(&empty);
  sem_destroy(&full);
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
    if (!(req.pref_seat_list[i] >= 1 &&
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
  fprintf(fslog, "%d-OPEN\n", (int)pthread_self());

  while (1)
  {
    sem_wait(&full);
    myreq = request;
    sem_post(&empty);

    int request_status = validate_request(myreq);
    switch (request_status)
    {
    case VALID_REQUEST:
    {
      // Check if room is available
      if (check_room_availability() == FULL_ROOM)
        exit(0);

      // Keeping track of booked seats
      int booked_seats_list[myreq.num_wanted_seats];
      for (int i = 0; i < myreq.num_wanted_seats; i++) {
        booked_seats_list[i] = -1;
      }
      for (int i = 0; i < myreq.num_wanted_seats; i++) {
        printf("book seats before init #%d\n", booked_seats_list[i]);
      }

      int booked_seats = 0;
      // Trying to book seats
      for (int i = 0; i < myreq.pref_seats_size; i++) {
        for (int j = 0; j < num_room_seats; j++) {
          if (myreq.pref_seat_list[i] == seats[j].number) {
            pthread_mutex_lock(&mutex);
            if (isSeatFree(&seats[j], myreq.pref_seat_list[i]) &&
                booked_seats < myreq.num_wanted_seats) {
              bookSeat(&seats[j], myreq.pref_seat_list[i], myreq.pid);
              booked_seats_list[booked_seats] = seats[j].number;
              booked_seats++;
            }
            pthread_mutex_unlock(&mutex);
          }
        }
      }
      if (booked_seats == myreq.num_wanted_seats)
        printf("Successful reservation\n\n");
      else {
        printf("Freeing seats because %d =/= %d\n", booked_seats, myreq.num_wanted_seats);
        free_booked_seats(booked_seats_list, myreq.num_wanted_seats);
      }

      break;
    }
    }
  }

  fprintf(fslog, "%d-CLOSED\n", (int)pthread_self());
}

void free_booked_seats(int booked_seats_list[], int num_wanted_seats) {

/*
  for (int i = 0; i < num_wanted_seats; i++) {
    printf("Readying to free seat #%d\n", booked_seats_list[i]);
  }
*/

  for (int i = 0; i < num_room_seats; i++) {
    for (int j = 0; j < num_wanted_seats; j++) {
      pthread_mutex_lock(&mutex);
      if (seats[i].number == booked_seats_list[j]) {
        freeSeat(&seats[i], booked_seats_list[j]);
      }
      pthread_mutex_unlock(&mutex);
    }
  }
}

int check_room_availability()
{
  int unavailable_seats = 0;
  for (int i = 0; i < num_room_seats; i++)
  {
    if (seats[i].available == SEAT_UNAVAILABLE)
      unavailable_seats++;
  }
  if (unavailable_seats == num_room_seats)
    return FULL_ROOM;
  return AVAILABLE_SEATS;
}

Seat *init_seats_list()
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
int isSeatFree(Seat *seat, int seatNum)
{
  if ((*seat).available == SEAT_AVAILABLE)
    return SEAT_AVAILABLE;
  else
    return SEAT_UNAVAILABLE;
}

// Booking seat seatnum for client clientId
void bookSeat(Seat *seat, int seatNum, int clientId)
{
  printf("Booking seat #%d\n", (*seat).number);
  (*seat).available = SEAT_UNAVAILABLE;
  (*seat).client_id = clientId;
}

// Freeing seat seatNum (in case the final reservation couldn't be done)
void freeSeat(Seat *seat, int seatNum)
{
  printf("Freeing seat #%d, requested %d\n", (*seat).number, seatNum);
  (*seat).available = SEAT_AVAILABLE;
  (*seat).client_id = -1;
}
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
#include <limits.h>

#include "request.h"
#include "seats.h"

void create_fifo_requests();
void get_client_requests(int open_time);
Seat *init_seats_list();

// General functions
void *safe_malloc(void *ptr, int nbytes);
char * stringify_list(int list[], int size);

// Thread functions
void launch_ticket_offices_threads();
void *ticket_office_handler(void *arg);
void free_booked_seats(int booked_seats_list[], int num_wanted_seats);
int isSeatFree(Seat *seat, int seatNum);
void bookSeat(Seat *seat, int seatNum, int clientId);
void freeSeat(Seat *seat, int seatNum);
int * init_booked_seats_list(int num_wanted_seats);
void print_request_error(FILE *f, int client_id, int thread_id, int n_seats, char *seats_list, char *error_code);
int validate_request(Request req);
void reply_to_client(int client_id, RequestReply reply);
void terminate_threads();

// Global variables
int num_room_seats; // Total number of room seats
Request request;    // Buffer of 1 unit to hold incoming request
Seat *seats;        // Array of seats (of type Seat)
int num_ticket_offices;
pthread_t *thread_ids;
int closed;

// Synchronization variables
sem_t empty, full; // global semaphores
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
  closed = 0;
  num_ticket_offices = strtol(argv[2], &end, 10);
  launch_ticket_offices_threads();

  /* Waiting for client requests */
  int open_time = strtol(argv[3], &end, 10);
  get_client_requests(open_time);

  // Destroying semaphores and mutex
  sem_destroy(&empty);
  sem_destroy(&full);
  pthread_mutex_destroy(&mutex);

  return 0;
}

/* Creates the fifo requests */
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

/* Allocates nbytes bytes of memory for the assigned pointer */
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

/* Listens to clients' requests during open_time seconds */
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
      // Synchronizing requests propagation to ticket offices (threads)
      sem_wait(&empty);
      request = req; // Assigning 1-unit buffer (request) to last received request
      sem_post(&full);
    }

    usleep(100000); // 100 ms
    current_time = time(NULL);
  }

  closed = 1;
  printf("Ticket offices closed.\n");
  unlink("requests");  
  terminate_threads();
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

void launch_ticket_offices_threads()
{
  // Initializing array of thread ids to terminate in the end
  thread_ids = safe_malloc(thread_ids, num_ticket_offices * sizeof(int));

  int thread_n[num_ticket_offices];
  pthread_t threads[num_ticket_offices];
  for (int i = 0; i < num_ticket_offices; i++)
  {
    thread_ids[i] = threads[i];
    thread_n[i] = i + 1;
    pthread_create(&threads[i], NULL, ticket_office_handler, &thread_n[i]);
  }
}

// Main ticket office functions
void *ticket_office_handler(void *arg)
{
  Request myreq;
  int thread_n = * (int *) arg;

  FILE *fslog, *fsbook;
  fslog = fopen(SLOG, "a");
  fsbook = fopen(SBOOK, "a");
  fprintf(fslog, "%d-OPEN\n", thread_n);
  /**
   * ALTERACAO: Adicionados fflush de forma a sincronizar a escrita nos ficheiros
   *            de acordo com a ordem das operacoes efetuadas
  **/
  fflush(fslog);

  while (closed == 0)
  {
    sem_wait(&full);
    myreq = request;
    sem_post(&empty);

    //printf("Inside thread #%d", thread_n);

    char * stringified_list;
    stringified_list = stringify_list(myreq.pref_seat_list, myreq.pref_seats_size);

    int request_status = validate_request(myreq);
    switch (request_status)
    {
    case VALID_REQUEST:
    {
      // Check if room is available
      if (check_room_availability() == FULL_ROOM)
        exit(0);

      // Keeping track of booked seats
      int booked_seats = 0;
      int booked_seats_list[myreq.num_wanted_seats];
      // Initializing the booked_seats_list (to -1)
      for (int i = 0; i < myreq.num_wanted_seats; i++) {
        booked_seats_list[i] = -1;
      }
      
      // Trying to book seats
      for (int i = 0; i < myreq.pref_seats_size; i++) {
        for (int j = 0; j < num_room_seats; j++) {
          if (myreq.pref_seat_list[i] == seats[j].number) {
            // Checking if seat is available to book
            if (isSeatFree(&seats[j], myreq.pref_seat_list[i]) &&
                booked_seats < myreq.num_wanted_seats) {
              // Locking while booking seats
              pthread_mutex_lock(&mutex);
              // Booking seat
              //DELAY(5);
              bookSeat(&seats[j], myreq.pref_seat_list[i], myreq.pid);
              // Unlocking after booking seats
              pthread_mutex_unlock(&mutex);
              // Updating array and counter of booked seats
              booked_seats_list[booked_seats] = seats[j].number;
              booked_seats++;
            }
          }
        }
      }
      // SUCCESSFUL RESERVATION
      if (booked_seats == myreq.num_wanted_seats) {
        char * stringified_booked_seats_list;
        // Stringifying booked seats list to simplify writing to file
        stringified_booked_seats_list = stringify_list(booked_seats_list, booked_seats);
        // Writing to server log file the request information
        fprintf(fslog, "%d-%d-%d: %s - %s\n", 
                      thread_n, 
                      myreq.pid, 
                      myreq.num_wanted_seats,
                      stringified_list,
                      stringified_booked_seats_list);
        /**
        * ALTERACAO: Adicionados fflush de forma a sincronizar a escrita nos ficheiros
        *            de acordo com a ordem das operacoes efetuadas
        **/
        fflush(fslog);
        // Writing to server booking the reserved seats
        for (int i = 0; i < booked_seats; i++) {
          fprintf(fsbook, "%d\n", booked_seats_list[i]);
          fflush(fsbook);
        }
        // Initializing reply
        RequestReply reply;
        reply.status = SUCCESSFUL_RESERVATION;
        reply.booked_seats_size = booked_seats;
        reply.client_wanted_seats = myreq.num_wanted_seats;
        // Initializing array to -1
        for (int i = 0; i < MAX_CLI_SEATS; i++) {
          reply.booked_seats[i] = -1;
        }
        // Assigning booked seats
        for (int i = 0; i < booked_seats; i++) {
          reply.booked_seats[i] = booked_seats_list[i];
        }
        // Replying to client
        printf("Successful reservation for client %d. Seats # %s\n", myreq.pid, stringified_booked_seats_list);
        reply_to_client(myreq.pid, reply);
      }
      // UNSUCCESSFUL RESERVATION
      else {
        // Freeing booked seats
        free_booked_seats(booked_seats_list, myreq.num_wanted_seats);
        // Printing to server log the request status
        print_request_error(fslog, myreq.pid, thread_n, myreq.num_wanted_seats, stringified_list, "NAV");
        // Initializing reply
        RequestReply reply;
        reply.status = UNSUCCESSFUL_RESERVATION;
        reply.error_code[0] = 'N';
        reply.error_code[1] = 'A';
        reply.error_code[2] = 'V';
        // Replying to client
        reply_to_client(myreq.pid, reply);
      }
      break;
    }
    case INVALID_PARAMETERS:
    {
      print_request_error(fslog, myreq.pid, thread_n, myreq.num_wanted_seats, stringified_list, "ERR");
      // Initializing reply
      RequestReply reply;
      reply.status = UNSUCCESSFUL_RESERVATION;
      reply.error_code[0] = 'E';
      reply.error_code[1] = 'R';
      reply.error_code[2] = 'R';
      // Replying to client
      reply_to_client(myreq.pid, reply);
      break;
    }
    case OVERFLOW_NUM_WANTED_SEATS:
    {
      print_request_error(fslog, myreq.pid, thread_n, myreq.num_wanted_seats, stringified_list, "MAX");
      // Initializing reply
      RequestReply reply;
      reply.status = UNSUCCESSFUL_RESERVATION;
      reply.error_code[0] = 'M';
      reply.error_code[1] = 'A';
      reply.error_code[2] = 'X';
      // Replying to client
      reply_to_client(myreq.pid, reply);
      break;
    }
    case INVALID_NUMBER_PREF_SEATS:
    {
      print_request_error(fslog, myreq.pid, thread_n, myreq.num_wanted_seats, stringified_list, "NST");
      // Initializing reply
      RequestReply reply;
      reply.status = UNSUCCESSFUL_RESERVATION;
      reply.error_code[0] = 'N';
      reply.error_code[1] = 'S';
      reply.error_code[2] = 'I';
      // Replying to client
      reply_to_client(myreq.pid, reply);
      break;
    }
    case INVALID_SEAT_NUMBER:
    {
      print_request_error(fslog, myreq.pid, thread_n, myreq.num_wanted_seats, stringified_list, "IID");
      // Initializing reply
      RequestReply reply;
      reply.status = UNSUCCESSFUL_RESERVATION;
      reply.error_code[0] = 'I';
      reply.error_code[1] = 'I';
      reply.error_code[2] = 'D';
      // Replying to client
      reply_to_client(myreq.pid, reply);
      break;
    }
    }
  }
  pthread_exit(0);
}

// Sends RequestReply struct to FIFO
void reply_to_client(int client_id, RequestReply reply)
{
  int fdans;
  char fifo_ans[MAX_FIFO_LENGTH];
  sprintf(fifo_ans, "ans%ld", (long)client_id);
  
  do
  {
    fdans = open(fifo_ans, O_WRONLY | O_NONBLOCK);
    if (fdans == -1)
      usleep(100000); // 100 ms
    
  } while (fdans == -1);

  write(fdans, &reply, sizeof(reply));

  //close(fdans);
}

// Transforms an array of ints into a string 
char *stringify_list(int list[], int size) 
{
  char *stringified_list;
  int nbytes = sizeof(char) * (4 * size + size - 1);
  stringified_list = safe_malloc(list, nbytes);
  
  char substr[4];
  for (int i = 0; i < size; i++) {
    int n = list[i];
    if (n < 10)
      sprintf(substr, " 000%d", list[i]);
    else if (n < 100)
      sprintf(substr, " 00%d", list[i]);
    else if (n < 1000)
      sprintf(substr, " 0%d", list[i]);
    else
      sprintf(substr, " %d", list[i]);
    strcat(stringified_list, substr);
    }
  return stringified_list;
}

void print_request_error(FILE *f, int client_id, int thread_id, int n_seats, char *seats_list, char *error_code) {
  printf("Error processing request from client #%d\n", client_id);
  fprintf(f, "%d-%d-%d: %s - %s\n", 
                      thread_id, 
                      client_id, 
                      n_seats,
                      seats_list,
                      error_code);
  /**
  * ALTERACAO: Adicionados fflush de forma a sincronizar a escrita nos ficheiros
  *            de acordo com a ordem das operacoes efetuadas
  **/                    
  fflush(f);
}

// Frees seats of the array booked_seats_list 
void free_booked_seats(int booked_seats_list[], int num_wanted_seats) 
{
  for (int i = 0; i < num_room_seats; i++) {
    for (int j = 0; j < num_wanted_seats; j++) {
      if (seats[i].number == booked_seats_list[j]) {
        // Locking while seats are being freed
        pthread_mutex_lock(&mutex);
        freeSeat(&seats[i], booked_seats_list[j]);
        pthread_mutex_unlock(&mutex);
        // Unlocking after seats are freed
      }
    }
  }
}

// Check if the room is not full
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

// Initializes the global array seats
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

// Testing if seat referenced by position seat is free
int isSeatFree(Seat *seat, int seatNum)
{
  if ((*seat).available == SEAT_AVAILABLE)
    return SEAT_AVAILABLE;
  else
    return SEAT_UNAVAILABLE;
}

// Booking seat referenced by position seat
void bookSeat(Seat *seat, int seatNum, int clientId)
{
  (*seat).available = SEAT_UNAVAILABLE;
  (*seat).client_id = clientId;
}

// Freeing seat referenced by position seat (in case final reservation can't be done)
void freeSeat(Seat *seat, int seatNum)
{
  (*seat).available = SEAT_AVAILABLE;
  (*seat).client_id = -1;
}

// Terminates running threads and writes their ending to file
/**
 * ALTERACAO: Adicionada string de terminacao das threads/bilheteiras
**/
void terminate_threads() {
  FILE *fslog;
  fslog = fopen(SLOG, "a");
  for (int i = 0; i < num_ticket_offices; i++) {
    fprintf(fslog, "%d-CLOSE\n", i + 1);
    /**
    * ALTERACAO: Adicionados fflush de forma a sincronizar a escrita nos ficheiros
    *            de acordo com a ordem das operacoes efetuadas
    **/
    fflush(fslog);
    /**
    * ALTERACAO: Removida espera pelas threads pois causava SEGFAULTs 
    *            por motivos desconhecidos
    **/
    //pthread_join(thread_ids[i], NULL);
  }
}
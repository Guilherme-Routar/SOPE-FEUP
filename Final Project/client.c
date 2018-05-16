#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include "macros.h"
#include "request.h"

#define REQUEST_TIMEOUT 30 // Number of seconds to wait for fifo request to open

void create_fifo_ans();
Request init_request(char *arglist[]);
void send_request(Request req);
void wait_answer(int timeout);
void write_to_clog(RequestReply reply);
void *safe_malloc(void *ptr, int nbytes);

int num_wanted_seats;

int main(int argc, char *argv[])
{
  /* Checking client command line arguments */
  if (argc != 4)
  {
    fprintf(stderr, "Usage: client <timeout> <num_wanted_seats> <pref_seat_list>");
    exit(0);
  }

  /* Creating answers fifo */
  create_fifo_ans();

  /* Sending server a request through FIFO requests */
  send_request(init_request(argv));

  /* Waiting for an answer from the server */
  char *end;
  int timeout = strtol(argv[1], &end, 10);
  wait_answer(timeout);

  return 0;
}

void create_fifo_ans()
{
  pid_t mypid = getpid();
  char fifo_ans[MAX_FIFO_LENGTH];
  sprintf(fifo_ans, "ans%ld", (long)mypid);

  if (mkfifo(fifo_ans, 0660) < 0)
    if (errno == EEXIST)
      printf("FIFO already created for the client with PID = %d\n", mypid);
    else
      printf("Can't create FIFO for the client with PID = %d\n", mypid);
  else
    printf("FIFO %s sucessfully created\n", fifo_ans);
}

int count_seats_list(char *seats_list)
{
  int size = 1;
  while ((seats_list = strchr(seats_list, ' ')) != NULL)
  {
    size++;
    seats_list++;
  }
  return size;
}

// Initializes request struct
Request init_request(char *arglist[])
{
  // Getting the size of pref_seat_list so we can initialize and allocate the array
  int pref_seats_size = count_seats_list(arglist[3]);

  Request req;

  // Initializing struct pref_seats_list
  for (int i = 0; i < MAX_CLI_SEATS; i++)
    req.pref_seat_list[i] = -1;

  // Getting client's PID
  req.pid = getpid();

  // Getting client's number of wanted seats
  char *end;
  req.num_wanted_seats = strtol(arglist[2], &end, 10);
  num_wanted_seats = req.num_wanted_seats;

  // Assigning size of the struct's pref_seat_list array
  req.pref_seats_size = pref_seats_size;

  // Assigning list of seats to struct's pref_seat_list array
  int i = 0;
  char *token = strtok(arglist[3], " ");
  while (token != NULL)
  {
    req.pref_seat_list[i++] = strtol(token, &end, 10);
    token = strtok(NULL, " ");
  }

  return req;
}

void send_request(Request req)
{
  // Attempting to open the fifo request
  int fdreq;
  int timespan = 0;
  do
  {
    timespan++;
    if (timespan == REQUEST_TIMEOUT)
    {
      fprintf(stderr, "FIFO request not open. Try again later.\n");
      exit(-1);
    }

    fdreq = open("requests", O_WRONLY | O_NONBLOCK);
    if (fdreq == -1)
      usleep(100000); // 100 ms

  } while (fdreq == -1);

  // Sending struct req to fifo requests
  write(fdreq, &req, sizeof(req));

  close(fdreq);
}

void wait_answer(int timeout)
{
  printf("Waiting for an answer..\n");

  pid_t mypid = getpid();
  char fifo_ans[MAX_FIFO_LENGTH];
  sprintf(fifo_ans, "ans%ld", (long)mypid);

  int fd_ans;

  if ((fd_ans = open(fifo_ans, O_RDONLY | O_NONBLOCK)) == -1)
  {
    fprintf(stderr, "Unable to open FIFO %s", fifo_ans);
    return;
  }

  printf("FIFO %s opened for reading\n", fifo_ans);

  time_t initial_time = time(NULL);
  time_t current_time = initial_time;
  while ((current_time - initial_time) < timeout)
  {
    RequestReply reply;
    int n;
    char str[25];
    n = read(fd_ans, &reply, sizeof(reply));
    if (n > 0)
    {
      write_to_clog(reply);
      unlink(fifo_ans);
      return;
    }
    usleep(100000); // 100 ms
    current_time = time(NULL);
  }
  printf("Time's up.");
  unlink(fifo_ans);
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

void write_to_clog(RequestReply reply)
{
  FILE *clog, *cbook;
  clog = fopen(CLOG, "a");
  cbook = fopen(CBOOK, "a");

  char str[2];
  if (reply.status == SUCCESSFUL_RESERVATION)
  {
    printf("Booked seats # ");

    char wanted_seats[2];
    if (reply.client_wanted_seats < 10)
      sprintf(wanted_seats, "0%d", reply.client_wanted_seats);
    else
      sprintf(wanted_seats, "%d", reply.client_wanted_seats);

    for (int i = 0; i < num_wanted_seats; i++)
    {
      char index[2];
      if (i < 10)
        sprintf(index, "0%d", i + 1);
      else
        sprintf(index, "%d", i + 1);

      char seat[4];
      if (reply.booked_seats[i] < 10)
        sprintf(seat, "000%d", reply.booked_seats[i]);
      else if (reply.booked_seats[i] < 100)
        sprintf(seat, "00%d", reply.booked_seats[i]);
      else if (reply.booked_seats[i] < 1000)
        sprintf(seat, "0%d", reply.booked_seats[i]);
      else
        sprintf(seat, "%d", reply.booked_seats[i]);

      printf("%d ", reply.booked_seats[i]); // Printing booked seats on the client side
      fprintf(clog, "%d %s.%s %s\n", getpid(), index, wanted_seats, seat);
      fflush(clog);
      fprintf(cbook, "%d\n", reply.booked_seats[i]);
      fflush(cbook);
    }
  }
  else if (reply.status = UNSUCCESSFUL_RESERVATION) {
    printf("Unsuccessful reservation: ERROR = %s", reply.error_code);
    fprintf(clog, "%d %s\n", getpid(), reply.error_code);
    fflush(clog);
  } 
}
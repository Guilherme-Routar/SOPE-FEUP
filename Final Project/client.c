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

#define MAX_FIFO_LENGTH 8 // Fifo's name max length
#define REQUEST_TIMEOUT 30 // Number of seconds to wait for fifo request to open

void create_fifo_ans();
Request init_request(char *arglist[]);
void send_request(Request req);
void wait_answer(int timeout);

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
  //wait_answer(timeout); 

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
    int n;
    char str[4];
    n = read(fd_ans, str, 100);
    if (n > 0)
    {
      printf("message = %s", str);
      return;
    }
    usleep(100000); // 100 ms
    current_time = time(NULL);
  }
  printf("Time's up.");
  unlink(fifo_ans);
}
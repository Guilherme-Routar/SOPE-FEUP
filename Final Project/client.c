#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "macros.h"
#include "request.h"

#define MAX_FIFO_LENGTH 8 // Fifo's name max length

void create_fifo_ans();
void send_request(char *arglist[]);

int main(int argc, char *argv[])
{
  /* Checking client command line arguments */
  if (argc != 4)
  {
    fprintf(stderr, "Usage: client <timeout> <num_wanted_seats> <pref_seat_list>");
    exit(0);
  }

  /* Creating answers fifo */
  //create_fifo_ans();

  /* Sending server a request through FIFO requests */
  send_request(argv);

  return 0;
}

void send_request(char *arglist[])
{
  struct request req;
  char *end;
  req.timeout = strtol(arglist[1], &end, 10);
  req.num_wanted_seats = strtol(arglist[2], &end, 10);

  // Initializing struct pref_seats_list
  for (int i = 0; i < MAX_CLI_SEATS; i++)
    req.pref_seat_list[i] = -1;

  // Assigning seats arguments list to struct's array
  // First n elements are seat numbers, the rest are -1
  // MAX_CLI_SEATS - n = number of "unassigned" seats (-1)
  int i = 0;
  char *p = strtok(arglist[3], " ");
  while (p != NULL)
  {
    req.pref_seat_list[i++] = strtol(p, &end, 10);
    p = strtok(NULL, " ");
  }

  // Attempting to open the fifo requests
  int fdreq;
  do
  {
    fdreq = open("requests", O_WRONLY);
    if (fdreq == -1)
      sleep(1);
  } while (fdreq == -1);
  // Sending struct request to fifo requests
  write(fdreq, &req, 101 * sizeof(int));

  close(fdreq); 
}

void create_fifo_ans()
{
  pid_t mypid = getpid();

  char fifo_ans[MAX_FIFO_LENGTH];
  sprintf(fifo_ans, "ans%ld", mypid);

  if (mkfifo(fifo_ans, 0660) < 0)
    if (errno == EEXIST)
      printf("FIFO already created for the client with PID = %d\n", mypid);
    else
      printf("Can't create FIFO for the client with PID = %d\n", mypid);
  else
    printf("FIFO %s sucessfully created\n", fifo_ans);
}

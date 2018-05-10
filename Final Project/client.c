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
#define REQUEST_TIMEOUT 5 // Number of seconds to wait for fifo request to open

void create_fifo_ans();
struct request parse_args(char *arglist[]);
void send_request(struct request req);
void wait_answer();

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
  send_request(parse_args(argv));

  /* Wait for an answer from the server */
  char *end;
  int timeout = strtol(argv[1], &end, 10);
  wait_answer(timeout);

  return 0;
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

struct request parse_args(char *arglist[])
{
  struct request req;
  char *end;
  req.pid = getpid();
  req.num_wanted_seats = strtol(arglist[2], &end, 10);

  // Initializing struct pref_seats_list
  for (int i = 0; i < MAX_CLI_SEATS; i++)
    req.pref_seat_list[i] = -1;

  // Assigning seats arguments list to struct's array
  // First n elements are seat numbers, the rest are -1
  // MAX_CLI_SEATS - n = number of "unassigned" seats (-1)
  int i = 0;
  char *token = strtok(arglist[3], " ");
  int seatnumber;
  while (token != NULL)
  {
    seatnumber = strtol(token, &end, 10);
    if (seatnumber > MAX_ROOM_SEATS) 
    {
      fprintf(stderr, "Unexistent seat number");
      exit(0);
    }
    req.pref_seat_list[i++] = seatnumber;
    token = strtok(NULL, " ");
  }

  return req;
}

void send_request(struct request req)
{
  // Attempting to open the fifo request
  int fdreq;
  int timespan = 0;
  do
  {
    if (timespan == REQUEST_TIMEOUT)
    {
      fprintf(stderr, "FIFO request not open. Try again later.\n");
      return;
    }
    fdreq = open("requests", O_WRONLY);
    if (fdreq == -1)
      sleep(1);
    timespan++;
  } while (fdreq == -1);

  // Sending struct req to fifo request
  write(fdreq, &req, 101 * sizeof(int));

  close(fdreq);
}

void wait_answer(int timeout)
{
  pid_t mypid = getpid();
  char fifo_ans[MAX_FIFO_LENGTH];
  sprintf(fifo_ans, "ans%ld", mypid);

  int fd_ans;
  if ((fd_ans = open(fifo_ans, O_RDONLY)) == -1)
  {
    char errorlog[50];
    fprintf(stderr, "Unable to open FIFO %s", fifo_ans);
    return;
  }

  printf("FIFO %s opened for reading\n", fifo_ans);

  int n; char str[100];
  n = read(fd_ans, str, 100);
  if (n > 0)
    printf("%s has arrived\n", str);
}
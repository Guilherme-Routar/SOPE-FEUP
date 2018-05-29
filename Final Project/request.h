#ifndef REQUEST_H_
#define REQUEST_H_

#include "macros.h"

typedef struct Requests
{
  pid_t pid;
  int num_wanted_seats;
  int pref_seats_size;
  int pref_seat_list[MAX_CLI_SEATS];
} Request;

typedef struct RequestReplies
{
  int status;
  int booked_seats_size;
  int client_wanted_seats;
  char error_code[3];
  int booked_seats[MAX_CLI_SEATS];
} RequestReply;

#endif
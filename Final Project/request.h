#ifndef REQUEST_H_
#define REQUEST_H_

#include "macros.h"

struct request {
  pid_t pid;
  int num_wanted_seats;
  int pref_seats_size;
  int pref_seat_list[MAX_CLI_SEATS];
};

#endif 
#ifndef REQUEST_H_
#define REQUEST_H_

struct request {
  pid_t pid;
  int num_wanted_seats;
  int pref_seat_list[MAX_CLI_SEATS];
};

#endif 
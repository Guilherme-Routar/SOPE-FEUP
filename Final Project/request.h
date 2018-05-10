#ifndef REQUEST_H_
#define REQUEST_H_

struct request {
  int timeout;
  int num_wanted_seats;
  int pref_seat_list[MAX_CLI_SEATS];
};

#endif 
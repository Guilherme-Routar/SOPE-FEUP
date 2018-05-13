#ifndef SEATS_H_
#define SEATS_H_

typedef struct Seats {
    int number;
    int available;
    pid_t client_id;
} Seat;

#endif
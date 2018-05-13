#ifndef SEATS_H_
#define SEATS_H_

#include <stdbool.h>

typedef struct Seats {
    int number;
    bool free;
    pid_t client_id;
} Seat;

#endif
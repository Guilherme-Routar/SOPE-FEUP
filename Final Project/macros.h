#ifndef MACROS_H_ 
#define MACROS_H_

#define SHARED 0

/* STANDARD MACROS */
#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define WIDTH_SEAT 4

/*  */
#define SEAT_AVAILABLE 11
#define SEAT_UNAVAILABLE 0

/* REQUESTS MACROS */
#define VALID_REQUEST 10
#define OVERFLOW_NUM_WANTED_SEATS -1
#define INVALID_NUMBER_PREF_SEATS -2
#define INVALID_SEAT_NUMBER -3
#define INVALID_PARAMETERS -4
#define UNAVAILABLE_SEATS -5
#define FULL_ROOM -6
#define AVAILABLE_SEATS 12

/* RESERVATION MACROS */
#define SUCCESSFUL_RESERVATION 1
#define UNSUCCESSFUL_RESERVATION -1

/* FILES */
#define SLOG "slog.txt"
#define CLOG "clog.txt"
#define SBOOK "sbook.txt"
#define CBOOK "cbook.txt"

/* FIFO NAME */
#define MAX_FIFO_LENGTH 8 // Fifo's name max length

/* OTHER ERRORS */
#define BAD_ALLOC NULL

#define DELAY(x) sleep(x)

#endif 
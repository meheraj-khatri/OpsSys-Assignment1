#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <semaphore.h>

// The table can only hold two items at the same time. 
#define BUFFER_SIZE 2

// Shared memory object name and semaphore names
#define SHM_NAME    "/PCProblem_SHM"
#define SEM_MUTEX   "/PCProblem_Mutex"
#define SEM_FULL    "/PCProblem_Full"
#define SEM_EMPTY   "/PCProblem_Empty"

// Structure that resides in shared memory
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;     // Index for the producer (next item to deposit)
    int out;    // Index for the consumer (next item to pick up)
    int counter; // Current number of items in the buffer
} shared_data_t;

// Total items to produce/consume
#define MAX_ITEMS 10

#endif // GLOBALS_H
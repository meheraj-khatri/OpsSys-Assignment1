#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

// --- Constants ---
#define BUFFER_SIZE 2             // The table can only hold two items [cite: 12]
#define SHM_NAME "/prod_con_shm"  // Name for the shared memory object
#define SEM_EMPTY "/sem_empty"    // Semaphore for empty slots (Producer waits here)
#define SEM_FULL "/sem_full"      // Semaphore for full slots (Consumer waits here)
#define SEM_MUTEX "/sem_mutex"    // Semaphore for mutual exclusion

// --- Shared Data Structure (The "Table") ---
typedef struct {
    int buffer[BUFFER_SIZE]; // The shared buffer
    int in;                  // Index for the producer
    int out;                 // Index for the consumer
} SharedBuffer;

// Structure to hold the shared memory pointer and semaphores
typedef struct {
    SharedBuffer *shm_ptr;
    sem_t *empty_sem;
    sem_t *full_sem;
    sem_t *mutex_sem;
} SharedResources;
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "global.h"

// Pointers to the shared resources
shared_data_t *shared_mem;
sem_t *mutex;
sem_t *full;    // Counting semaphore: Number of occupied slots (items available for consumption)
sem_t *empty;   // Counting semaphore: Number of empty slots (slots available for production)

// Thread function for the producer
void* producer_thread(void* arg) {
    int item_produced;
    int items_count = 0;
    
    // Seed the random number generator
    srand(time(NULL) + getpid());

    while (items_count < MAX_ITEMS) {
        
        // Wait for an empty slot (wait if buffer is full) 
        sem_wait(empty);
        
        // Wait for mutual exclusion (critical section entry) 
        sem_wait(mutex);

        // --- Critical Section Start ---
        
        // 1. Generate item
        item_produced = rand() % 100 + 1; // Generate a random item
        
        // 2. Put item onto the table
        shared_mem->buffer[shared_mem->in] = item_produced;
        printf("Producer: Produced %d at index %d. Buffer count: %d\n", 
               item_produced, shared_mem->in, shared_mem->counter + 1);
        fflush(stdout);

        // 3. Update indices and counter
        shared_mem->in = (shared_mem->in + 1) % BUFFER_SIZE;
        shared_mem->counter++;
        items_count++;

        // --- Critical Section End ---
        
        // Signal that mutual exclusion is released
        sem_post(mutex);
        
        // Signal that a slot is full (item is ready for consumption)
        sem_post(full);
        
        // Wait a short, random amount of time before producing the next item
        usleep(rand() % 500000 + 100000); 
    }

    printf("\n--- Producer is done producing %d items. ---\n", MAX_ITEMS);
    return NULL;
}


int main() {
    int shm_fd;

    // 1. Create and configure shared memory object
    // O_CREAT: Create the object if it does not exist
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(1);
    }
    
    // Configure the size of the shared memory segment
    ftruncate(shm_fd, sizeof(shared_data_t));

    // Map the shared memory segment to the address space of the process
    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // 2. Initialize the shared structure data
    shared_mem->in = 0;
    shared_mem->out = 0;
    shared_mem->counter = 0;

    // 3. Create and initialize named semaphores (O_CREAT)
    
    // Mutex (Initial value 1 for mutual exclusion) 
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    
    // 'full' (Initial value 0, as the buffer starts empty) 
    full = sem_open(SEM_FULL, O_CREAT, 0666, 0);
    
    // 'empty' (Initial value BUFFER_SIZE, as all slots are initially available) 
    empty = sem_open(SEM_EMPTY, O_CREAT, 0666, BUFFER_SIZE);

    if (mutex == SEM_FAILED || full == SEM_FAILED || empty == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // 4. Create producer thread and wait for it to finish
    pthread_t prod_thread;
    pthread_create(&prod_thread, NULL, producer_thread, NULL);
    pthread_join(prod_thread, NULL);

    // 5. Cleanup Resources (Producer takes ownership of unlinking)
    
    // Close the semaphore descriptors
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    
    // Unlink semaphores to destroy them from the system
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_EMPTY);
    
    // Unmap the shared memory segment
    munmap(shared_mem, sizeof(shared_data_t));
    
    // Close the file descriptor
    close(shm_fd);
    
    // Unlink the shared memory object
    shm_unlink(SHM_NAME);

    return 0;
}
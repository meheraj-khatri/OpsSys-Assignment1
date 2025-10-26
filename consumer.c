#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <pthread.h>

#include "global.h"

// Pointers to the shared resources
shared_data_t *shared_mem;
sem_t *mutex;
sem_t *full;    // Number of occupied slots
sem_t *empty;   // Number of empty slots

// Thread function for the consumer
void* consumer_thread(void* arg) {
    int item_consumed;
    int items_count = 0;

    while (items_count < MAX_ITEMS) {
        
        // Wait for a full slot (wait if buffer is empty) 
        sem_wait(full);
        
        // Wait for mutual exclusion (critical section entry) 
        sem_wait(mutex);

        // --- Critical Section Start ---
        
        // 1. Pick up item
        item_consumed = shared_mem->buffer[shared_mem->out];
        printf("Consumer: Consumed %d at index %d. Buffer count: %d\n", 
               item_consumed, shared_mem->out, shared_mem->counter - 1);
        fflush(stdout);

        // 2. Update indices and counter
        shared_mem->out = (shared_mem->out + 1) % BUFFER_SIZE;
        shared_mem->counter--;
        items_count++;
        
        // --- Critical Section End ---
        
        // Signal that mutual exclusion is released
        sem_post(mutex);
        
        // Signal that a slot is empty (slot is ready for production)
        sem_post(empty);
        
        // Wait a short, random amount of time before consuming the next item
        usleep(rand() % 600000 + 50000);
    }

    printf("\n--- Consumer is done consuming %d items. ---\n", MAX_ITEMS);
    return NULL;
}


int main() {
    int shm_fd;

    // Give producer time to initialize resources (wait 1 second)
    sleep(1);

    // 1. Open shared memory object (O_RDWR, no O_CREAT)
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed. Did producer run first?");
        exit(1);
    }

    // Map the shared memory segment
    shared_mem = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // 2. Open named semaphores (no O_CREAT, second param 0)
    mutex = sem_open(SEM_MUTEX, 0);
    full = sem_open(SEM_FULL, 0);
    empty = sem_open(SEM_EMPTY, 0);

    if (mutex == SEM_FAILED || full == SEM_FAILED || empty == SEM_FAILED) {
        perror("sem_open failed. Did producer initialize semaphores?");
        exit(1);
    }

    // 3. Create consumer thread and wait for it to finish
    pthread_t cons_thread;
    pthread_create(&cons_thread, NULL, consumer_thread, NULL);
    pthread_join(cons_thread, NULL);

    // 4. Cleanup Resources (Consumer only closes and unmaps)
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    
    munmap(shared_mem, sizeof(shared_data_t));
    close(shm_fd);

    return 0;
}
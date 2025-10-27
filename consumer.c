#include "global.h"

// Global shared resources
SharedResources resources;

// The thread function for the consumer
void *consumer_thread(void *arg) {
    SharedBuffer *shm = resources.shm_ptr;

    while (1) { // Runs continuously until manually stopped
        // 1. Wait for a full slot (wait if buffer is empty)
        // When there are no items, the consumer will wait. [cite: 13]
        if (sem_wait(resources.full_sem) == -1) {
            perror("Consumer: sem_wait(full) failed");
            break;
        }

        // 2. Wait for mutual exclusion (lock the buffer)
        if (sem_wait(resources.mutex_sem) == -1) {
            perror("Consumer: sem_wait(mutex) failed");
            sem_post(resources.full_sem); 
            break;
        }

        // Critical Section: Consume item
        int item = shm->buffer[shm->out];
        printf("Consumer consumed item: %d from index %d\n", item, shm->out);
        shm->buffer[shm->out] = 0; // Clear the slot
        shm->out = (shm->out + 1) % BUFFER_SIZE;

        // 3. Signal mutual exclusion (unlock the buffer)
        if (sem_post(resources.mutex_sem) == -1) {
            perror("Consumer: sem_post(mutex) failed");
            break;
        }

        // 4. Signal an empty slot (increase count of empty slots)
        if (sem_post(resources.empty_sem) == -1) {
            perror("Consumer: sem_post(empty) failed");
            break;
        }

        // Simulate work time
        usleep(rand() % 1000000 + 500000); // Sleep for 0.5 to 1.5 seconds
    }
    pthread_exit(NULL);
}

// Function to clean up shared resources (only closing handles here)
void cleanup() {
    sem_close(resources.empty_sem);
    sem_close(resources.full_sem);
    sem_close(resources.mutex_sem);
    munmap(resources.shm_ptr, sizeof(SharedBuffer));
    printf("\nConsumer cleanup complete.\n");
}
int main() {
    srand(time(NULL) * getpid()); // Seed for random numbers
    sleep(1); // Give producer time to create resources

    // --- 1. Link to Shared Memory ---
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open failed. Ensure producer is running first."); exit(EXIT_FAILURE); }
    resources.shm_ptr = (SharedBuffer *)mmap(0, sizeof(SharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    
    // --- 2. Link to Semaphores ---
    resources.empty_sem = sem_open(SEM_EMPTY, 0); 
    resources.full_sem  = sem_open(SEM_FULL, 0);
    resources.mutex_sem = sem_open(SEM_MUTEX, 0);

    // --- 3. Start Consumer Thread ---
    pthread_t con_tid;
    printf("Starting Consumer process...\n");

    if (pthread_create(&con_tid, NULL, consumer_thread, NULL) != 0) {
        perror("pthread_create failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    pthread_join(con_tid, NULL); // Wait for the thread (will wait indefinitely)
    cleanup();
    return 0;
}
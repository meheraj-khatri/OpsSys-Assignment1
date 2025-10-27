#include "global.h"

// Global shared resources
SharedResources resources;

// The thread function for the producer
void *producer_thread(void *arg) {
    int item_counter = 0;
    SharedBuffer *shm = resources.shm_ptr;

    while (1) { // Runs continuously until manually stopped
        //Wait for an empty slot (wait if buffer is full)
        // When the table is completed, the producer will wait.
        if (sem_wait(resources.empty_sem) == -1) {
            perror("Producer: sem_wait(empty) failed");
            break;
        }

        // Wait for mutual exclusion (lock the buffer)
        if (sem_wait(resources.mutex_sem) == -1) {
            perror("Producer: sem_wait(mutex) failed");
            sem_post(resources.empty_sem); 
            break;
        }

        // Critical Section: Produce item
        int item = ++item_counter;
        // Generate a random-looking number instead of just the counter for better demo
        item = (rand() % 900) + 100; 
        shm->buffer[shm->in] = item;
        printf("Producer produced item: %d at index %d\n", item, shm->in);
        shm->in = (shm->in + 1) % BUFFER_SIZE;

        // Signal mutual exclusion (unlock the buffer)
        if (sem_post(resources.mutex_sem) == -1) {
            perror("Producer: sem_post(mutex) failed");
            break;
        }

        // Signal a full slot (increase count of items)
        if (sem_post(resources.full_sem) == -1) {
            perror("Producer: sem_post(full) failed");
            break;
        }

        // Simulate work time (optional, but helps demonstrate interleaving)
        usleep(rand() % 1000000 + 500000); // Sleep for 0.5 to 1.5 seconds
    }
    pthread_exit(NULL);
}

// Function to clean up shared resources before exit
void cleanup() {
    // Close and Unlink semaphores and shared memory (best done by the creator)
    sem_close(resources.empty_sem);
    sem_close(resources.full_sem);
    sem_close(resources.mutex_sem);
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_MUTEX);
    munmap(resources.shm_ptr, sizeof(SharedBuffer));
    shm_unlink(SHM_NAME);
    printf("\nProducer cleanup complete.\n");
}

int main() {
    srand(time(NULL) * getpid()); // Seed for random numbers
    // Setup Shared Memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open failed"); exit(EXIT_FAILURE); }
    if (ftruncate(shm_fd, sizeof(SharedBuffer)) == -1) { perror("ftruncate failed"); close(shm_fd); exit(EXIT_FAILURE); }
    resources.shm_ptr = (SharedBuffer *)mmap(0, sizeof(SharedBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    // Initialize the shared memory structure
    memset(resources.shm_ptr, 0, sizeof(SharedBuffer));
    resources.shm_ptr->in = 0;
    resources.shm_ptr->out = 0;
    
    // Setup Semaphores (Unlink first for clean start)
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);
    sem_unlink(SEM_MUTEX);
    
    // Initial values: empty = BUFFER_SIZE (2 slots empty), full = 0 (0 items), mutex = 1 (unlocked)
    resources.empty_sem = sem_open(SEM_EMPTY, O_CREAT, 0666, BUFFER_SIZE);
    resources.full_sem  = sem_open(SEM_FULL, O_CREAT, 0666, 0);
    resources.mutex_sem = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

    // Start Producer Thread
    pthread_t prod_tid;
    printf("Starting Producer process with buffer size %d...\n", BUFFER_SIZE);

    if (pthread_create(&prod_tid, NULL, producer_thread, NULL) != 0) {
        perror("pthread_create failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    pthread_join(prod_tid, NULL); // Wait for the thread (will wait indefinitely)
    cleanup();
    return 0;
}
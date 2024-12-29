#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "shared_memory.h"
#include <semaphore.h>

#define SHM_NAME "/shared_memory"
#define SHM_SIZE sizeof(SharedMemory)

SharedMemory* shm_ptr = NULL;
int shm_fd = -1;

// Cleanup function
void cleanup(const char* shmid) {
    printf("Cleaning up resources...\n");

    // Destroy semaphores
    if (shm_ptr) {
        for (int i = 0; i < NUM_TABLES; i++) {
            if (sem_destroy(&shm_ptr->table_sems[i]) == -1) {
                perror("sem_destroy failed for table_sems");
            }
        }

        if (sem_destroy(&shm_ptr->logging) == -1) {
            perror("sem_destroy failed for logging semaphore");
        }

        if (sem_destroy(&shm_ptr->shm_mutex) == -1) {
            perror("sem_destroy failed for shm_mutex");
        }
    }

    // Unlink shared memory
    if (shm_unlink(shmid) == -1) {
        perror("shm_unlink failed");
    }

    // Unmap shared memory
    if (shm_ptr) {
        if (munmap(shm_ptr, SHM_SIZE) == -1) {
            perror("munmap failed");
        }
    }

    printf("Cleanup completed.\n");
    exit(0);
}

// Signal handler for cleanup
void handle_signal(int sig) {
    if (sig == SIGINT) {
        cleanup(SHM_NAME);
    }
}

int main(int argc, char* argv[]) {
    int ordertime = 0;
    char* shmid = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            ordertime = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0) {
            shmid = argv[++i];
        }
    }

    if (!shmid || ordertime <= 0) {
        printf("Usage: ./receptionist -d ordertime -s shmid\n");
        return 1;
    }

    // Register signal handler for cleanup
    signal(SIGINT, handle_signal);

    // Create shared memory
    shm_fd = shm_open(shmid, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // Initialize shared memory
    memset(shm_ptr, 0, SHM_SIZE);

    for (int table = 0; table < NUM_TABLES; table++) {
        shm_ptr->tables[table].blocked = false;
        shm_ptr->tables[table].num_occupied = 0;
        for (int chair = 0; chair < CHAIRS_PER_TABLE; chair++) {
            shm_ptr->tables[table].chairs[chair].occupied_by_pid = 0;
        }
    }

    printf("Shared memory initialized with name: %s\n", shmid);

    // Initialize semaphores
    if (sem_init(&shm_ptr->shm_mutex, 1, 1) == -1) { // Shared between processes
        perror("sem_init failed for shm_mutex");
        cleanup(shmid);
    }

    for (int i = 0; i < NUM_TABLES; i++) {
        if (sem_init(&shm_ptr->table_sems[i], 1, 1) == -1) { // Shared between processes
            perror("sem_init failed for table_sems");
            cleanup(shmid);
        }
    }

    if (sem_init(&shm_ptr->logging, 1, 1) == -1) {
        perror("sem_init failed for logging semaphore");
        cleanup(shmid);
    }

    if (sem_init(&shm_ptr->wakeup, 1, 0) == -1) { // Initial value 0
        perror("sem_init failed for wakeup semaphore");
        cleanup(shmid);
    }

    // Simulate receptionist work
    // while (1) {
    //     printf("Receptionist working... Sleeping for %d seconds.\n", ordertime);
    //     sleep(ordertime);
    // }

    while (1) {
        printf("Receptionist is waiting for visitors...\n");
        sem_wait(&shm_ptr->wakeup); // Wait for a visitor to wake up the receptionist

        printf("Receptionist is processing an order...\n");
        sleep(ordertime); // Simulate processing time

        printf("Receptionist finished processing an order.\n");
    }

    return 0;
}

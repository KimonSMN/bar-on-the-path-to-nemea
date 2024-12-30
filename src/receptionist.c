#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "shared_memory.h"
#include <semaphore.h>
#include <time.h>

#define SHM_NAME "/shared_memory"
#define SHM_SIZE sizeof(SharedMemory)

SharedMemory* shm_ptr = NULL;
int shm_fd = -1;

void cleanup(char* shmid) {
    printf("Cleaning up resources...\n");

    // Destroy semaphores
    if (shm_ptr) {
        for (int i = 0; i < NUM_TABLES; i++) {
            sem_destroy(&shm_ptr->table_sems[i]);
        }
        sem_destroy(&shm_ptr->logging);
    }

    shm_unlink(shmid);
    munmap(shm_ptr, SHM_SIZE);

    printf("Cleanup completed.\n");
    exit(0);
}

// Signal handle for ctrl + c.
void handle_signal(int sig) {
    if (sig == SIGINT) {
        printf("Open hours have ended. Closing receptionist.\n");
        printf("Total Visitors: %d\n",shm_ptr->total_visitors);
        float avg_rest = (float)shm_ptr->avg_rest_time / (float)shm_ptr->total_visitors;
        printf("Average Rest time: %f\n", avg_rest);
        float avg_wait = (float)shm_ptr->avg_wait_time / (float)shm_ptr->total_visitors;
        printf("Average Wait time: %f\n", avg_wait);
        printf("Orders made today: %d\n", shm_ptr->order_count);
        cleanup(SHM_NAME);
    }
}

int main(int argc, char* argv[]) {
    int ordertime = 0;  // Max time processing an order.
    int open_hours = 0;
    char* shmid = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            ordertime = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0) {
            shmid = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0){
            open_hours = atoi(argv[++i]);
        }
    }

    if (!shmid || ordertime <= 0) {
        printf("Usage: ./receptionist [OPTION]\n\t-d ordertime\tMMax time processing an order (seconds).\n\t-s shmid\tShared memory key.\n\t-o open_hours\t Max time until shop closure (in seconds).\n");
        return 1;
    }

    // Register signal handler for cleanup
    signal(SIGINT, handle_signal);

    // Create shared memory
    shm_fd = shm_open(shmid, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        return 1;
    }

    shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        return 1;
    }

    memset(shm_ptr, 0, SHM_SIZE);

    for (int table = 0; table < NUM_TABLES; table++) {
        shm_ptr->tables[table].blocked = false;
        shm_ptr->tables[table].num_occupied = 0;
        for (int chair = 0; chair < CHAIRS_PER_TABLE; chair++) {
            shm_ptr->tables[table].chairs[chair].occupied_by_pid = 0;
        }
    }
    shm_ptr->avg_rest_time = 0.0;
    shm_ptr->total_visitors = 0;
    shm_ptr->order_count = 0;

    printf("Shared memory initialized & named: %s\n", shmid);

    for (int i = 0; i < NUM_TABLES; i++) {
        if (sem_init(&shm_ptr->table_sems[i], 1, 1) == -1) {
            cleanup(shmid);
        }
    }

    if (sem_init(&shm_ptr->logging, 1, 1) == -1) {
        cleanup(shmid);
    }

    if (sem_init(&shm_ptr->wakeup, 1, 0) == -1) {
        cleanup(shmid);
    }

    time_t start_time = time(NULL); // Time used for open_hours.

    while (1) {
        time_t elapsed_time = time(NULL) - start_time;
        // printf("Elapsed time: %ld\n", elapsed_time); // Debugging.

        if (elapsed_time >= open_hours) {
            printf("Open hours have ended. Closing receptionist.\n");
            printf("Total Visitors: %d\n",shm_ptr->total_visitors);
            float avg_rest = (float)shm_ptr->avg_rest_time / (float)shm_ptr->total_visitors;
            printf("Average Rest time: %f\n", avg_rest);
            float avg_wait = (float)shm_ptr->avg_wait_time / (float)shm_ptr->total_visitors;
            printf("Average Wait time: %f\n", avg_wait);
            printf("Orders made today: %d\n", shm_ptr->order_count);
            cleanup(shmid);
        }
        printf("Receptionist is waiting for visitors...\n");
        sem_wait(&shm_ptr->wakeup); // Wait for a visitor to wake up the receptionist

        printf("Receptionist is processing an order...\n");
        sleep(ordertime); // Simulate processing time

        printf("Receptionist finished processing an order.\n");
    }

    return 0;
}

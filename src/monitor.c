#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared_memory.h"

#define SHM_NAME "/shared_memory"
#define SHM_SIZE sizeof(SharedMemory)

void display_bar_state(SharedMemory* shm_ptr) {
    printf("=== Current state of Bar ===\n");
    for (int table = 0; table < NUM_TABLES; table++) {
        if(shm_ptr->tables[table].blocked){
            printf("Table %d: Blocked\n", table);
        } else{
            printf("Table %d: Unblocked\n", table);
        }
        printf("  Chairs:\n");
        for (int chair = 0; chair < CHAIRS_PER_TABLE; chair++) {
            if (shm_ptr->tables[table].chairs[chair].occupied_by_pid == 0) {
                printf("\tChair %d: Unoccupied\n", chair);
            } else {
                printf("\tChair %d: Occupied by PID %d\n", chair, shm_ptr->tables[table].chairs[chair].occupied_by_pid);
            }
        }
    }
    printf("\n=== Total Visitors ===\n");
    printf("Visitors: %d\n",shm_ptr->total_visitors);


    printf("\n=== Products Consumed ===\n");
    printf("Water: %d\n", shm_ptr->product_stats[WATER]);
    printf("Wine: %d\n", shm_ptr->product_stats[WINE]);
    printf("Cheese: %d\n", shm_ptr->product_stats[CHEESE]);
    printf("Salad: %d\n", shm_ptr->product_stats[SALAD]);
}

int main(int argc, char* argv[]) {
    char* shmid = NULL;

    // Parse flags.
    if (strcmp(argv[1], "-s") == 0) {
        shmid = argv[2];
    }
    
    int shm_fd = shm_open(shmid, O_RDWR, 0666);
    if (shm_fd == -1) { // Error checking.
        return 1;
    }

    SharedMemory* shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) { // Error checking.
        return 1;
    }

    display_bar_state(shm_ptr);
    return 0;
}
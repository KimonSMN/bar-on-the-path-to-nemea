#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shared_memory.h"

#define SHM_SIZE sizeof(SharedMemory)

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

    // Create shared memory
    int shm_fd = shm_open(shmid, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    SharedMemory* shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    printf("Shared memory initialized with name: %s\n", shmid);

    // Initialize shared memory
    memset(shm_ptr, 0, SHM_SIZE);

    return 0;
}

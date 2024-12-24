#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared_memory.h"

#define SHM_NAME "/shared_memory"
#define SHM_SIZE sizeof(SharedMemory)

int main(int argc, char* argv[]){

  //////// HANDLE COMMAND LINE ARGUMENTS ////////
    int resttime;   // παρέχει την μέγιστη δυνατή περίοδο στην οποία ο επισκέπτης παραμένει στο τραπέζι αφότου έχει εξυπηρετηθεί από τον υπεύθυνο (σε κατάσταση sleep).
    char* shmid = NULL;      // δίνει το κλειδί που το κοινό τμήμα μνήμης έχει (και όπου υπάρχουν αντικείμενα κοινού ενδιαφέροντος).

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            resttime = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0) {
            shmid = argv[++i];
        } else {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }

    if(argc == 1){
        printf("Please provide -d & -s flags\n");
        return 0;
    }

    // if (!shmid || resttime <= 0) {
    //     printf("Usage: ./visitor -d resttime -s shmid\n");
    //     return 1;
    // }

    // Attach to shared memory
    int shm_fd = shm_open(shmid, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return 1;
    }

    SharedMemory* shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    printf("resttime: %d\n", resttime);
    printf("shmid: %s\n", shmid);


    // Check if there is empty chair at table
}
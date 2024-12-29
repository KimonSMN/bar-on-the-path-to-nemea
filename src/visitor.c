#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shared_memory.h"

#define SHM_NAME "/shared_memory"
#define SHM_SIZE sizeof(SharedMemory)

int check_for_chair(SharedMemory* shm_ptr, int* found_table, int* found_chair) {
    for (int table = 0; table < NUM_TABLES; table++) {
        if (shm_ptr->tables[table].blocked && shm_ptr->tables[table].num_occupied > 0) {
            continue; // Skip blocked tables
        }
        for (int chair = 0; chair < CHAIRS_PER_TABLE; chair++) {
            if (shm_ptr->tables[table].chairs[chair].occupied_by_pid == 0) {
                shm_ptr->tables[table].chairs[chair].occupied_by_pid = getpid(); // Occupy chair
                shm_ptr->tables[table].num_occupied++;
                if (shm_ptr->tables[table].num_occupied == CHAIRS_PER_TABLE) {
                    shm_ptr->tables[table].blocked = true; // Block the table
                }
                // shm_ptr->total_visitors++;
                *found_table = table;
                *found_chair = chair;
                return 1; // Chair found
            }
        }
    }
    return 0; // No chair found
}
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

   int found_table = -1, found_chair = -1;
    while (!check_for_chair(shm_ptr, &found_table, &found_chair)) {
        printf("Visitor %d: No chair found. Waiting...\n", getpid());
        sleep(2); // Wait before retrying
    }

    printf("Visitor %d: Seated at table %d, chair %d.\n", getpid(), found_table, found_chair);

    // Orders from the menu

    VisitorOrder order = {0};

    srand(time(NULL) + getpid());
    
    clock_t order_time = clock();

    int get_both, option;
    get_both = rand() % 2;
    if(get_both == 1){ // Get both
        order.orders[WATER] = 1;
        order.orders[WINE] = 1;
    } else{ // Don't get both
        option = rand() % 2;
        if(option == 1){ // Get Water
            order.orders[WATER] = 1;
            order.orders[WINE] = 0;
        } else{ // Get Wine
            order.orders[WATER] = 0;
            order.orders[WINE] = 1;
        } 
    }
    order.orders[CHEESE] = rand() % 2;  // Get Cheese
    order.orders[SALAD] = rand() % 2;   // Get Salad

    order_time = clock();

    printf("Visitor %d ordered: Water=%d, Wine=%d, Cheese=%d, Salad=%d, Order time:%ld\n",
       getpid(), order.orders[WATER], order.orders[WINE], 
       order.orders[CHEESE], order.orders[SALAD], order_time);

    // Visitor leaves after resttime
    printf("Visitor %d: Resting for %d seconds...\n", getpid(), resttime);
    sleep(resttime);

    shm_ptr->tables[found_table].chairs[found_chair].occupied_by_pid = 0; // Free chair
    shm_ptr->tables[found_table].num_occupied--;

    if (shm_ptr->tables[found_table].num_occupied == 0) {
        shm_ptr->tables[found_table].blocked = false; // Unblock the table
    }    
    printf("Visitor %d: Left table %d, chair %d.\n", getpid(), found_table, found_chair);

    munmap(shm_ptr, SHM_SIZE);


}
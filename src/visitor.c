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

// Function to check and possibly occupy a chair by a visitor.
int check_for_chair(SharedMemory* shm_ptr, int* found_table, int* found_chair) {   
    for (int table = 0; table < NUM_TABLES; table++) {
        sem_wait(&shm_ptr->table_sems[table]); // Lock table semaphore.

        if (shm_ptr->tables[table].blocked && shm_ptr->tables[table].num_occupied > 0) {
            sem_post(&shm_ptr->table_sems[table]); // Unlock before skipping.
            continue; // Skip blocked tables.
        }

        for (int chair = 0; chair < CHAIRS_PER_TABLE; chair++) {
            if (shm_ptr->tables[table].chairs[chair].occupied_by_pid == 0) {     // If chair is empty. (occupied_by_pid == 0)
                shm_ptr->tables[table].chairs[chair].occupied_by_pid = getpid(); // Set chair pid to visitor pid.
                shm_ptr->tables[table].num_occupied++;                           // Increment the number of chairs per tables that are occupied currently.
                if (shm_ptr->tables[table].num_occupied == CHAIRS_PER_TABLE) {   // If all the chairs are occupied, block the table.
                    shm_ptr->tables[table].blocked = true;  // Table blocked.
                }
                shm_ptr->total_visitors++;  // Increment the total visitors counter.
                *found_table = table;       // Save table &
                *found_chair = chair;       // Chair for future use.
                sem_post(&shm_ptr->wakeup); // Wake up receptionist.

                sem_post(&shm_ptr->table_sems[table]); // Unlock table semaphore.
                return 1; // Chair found.
            }
        }

        sem_post(&shm_ptr->table_sems[table]); // Unlock table semaphore.
    }
    return 0; // Chair not found.
}

int main(int argc, char* argv[]){

  //////// HANDLE COMMAND LINE ARGUMENTS ////////
    int resttime;           // Max time a visitor will stay(rest) at a table (sleep).
    char* shmid = NULL;     // Shared memory key.

    // Parse the flags.
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

    // Error checking.
    if (!shmid || resttime <= 0) {
        printf("Usage: ./visitor [OPTION]\n\t-d resttime\tMax time a visitor will rest (seconds).\n\t-s shmid\tShared memory key.\n");
        return 1;
    }

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

    clock_t start_wait_time, end_wait_time;
    int padding = 0;
    int found_table = -1, found_chair = -1;
    start_wait_time = clock();
    while (!check_for_chair(shm_ptr, &found_table, &found_chair)) {
        printf("Visitor %d: No chair found. Waiting...\n", getpid());
        sleep(2);
        padding += 2;   // Added a padding to add on the wait calculation for +2 seconds because of the sleep function.
    }
    end_wait_time = clock();
    double wait_time_in_seconds = padding + ((double) (end_wait_time - start_wait_time)) / CLOCKS_PER_SEC;

    sem_wait(&shm_ptr->logging); // Lock logging semaphore.
    shm_ptr->avg_wait_time += wait_time_in_seconds;
    sem_post(&shm_ptr->logging); // Unlock logging semaphore.

    printf("Visitor %d: Seated at table %d, chair %d. It took: %f\n", getpid(), found_table, found_chair, wait_time_in_seconds);
    
    srand(time(NULL) + getpid());   

    //======================================Debugging============================================================//
    // Initalize clocks to measure average order time.
    // clock_t start_order_time, end_order_time;
    // start_order_time = clock(); // Start of order_time. 
    //===========================================================================================================//

    // Visitor's Order.
    VisitorOrder order = {0};
    int get_both, option;
    get_both = rand() % 2;
    if(get_both == 1){  // Get both
        order.orders[WATER] = 1;
        order.orders[WINE] = 1;
    } else{             // Don't get both
        option = rand() % 2;
        if(option == 1){ // Get Water
            order.orders[WATER] = 1;
            order.orders[WINE] = 0;
        } else{          // Get Wine
            order.orders[WATER] = 0;
            order.orders[WINE] = 1;
        } 
    }
    order.orders[CHEESE] = rand() % 2;  // Get Cheese
    order.orders[SALAD] = rand() % 2;   // Get Salad

    // Logging orders to shared memory.
    sem_wait(&shm_ptr->logging); // Lock semaphore for logging.
    for (int i = 0; i < NUM_MENU_ITEMS; i++) {
        shm_ptr->product_stats[i] += order.orders[i]; // Update product stats for each item.
    }
    shm_ptr->order_count++;
    sem_post(&shm_ptr->logging); // Unlock semaphore for logging.

    //======================================Debugging============================================================//
    // end_order_time = clock(); // End of order_time.  // Debugging.
    // double time_in_seconds = ((double) (end_order_time - start_order_time)) / CLOCKS_PER_SEC; // Debugging.
    // printf("Visitor %d: Ordered: Water=%d, Wine=%d, Cheese=%d, Salad=%d, Order time:%f\n",  // Debugging.
    //    getpid(), order.orders[WATER], order.orders[WINE], 
    //    order.orders[CHEESE], order.orders[SALAD], time_in_seconds);
    // printf("Visitor %d: Resting for %d seconds...\n", getpid(), resttime);   // Debugging.
    //===========================================================================================================//
    // Visitor rests.
    sleep(resttime);

    // Updating average resttime.
    sem_wait(&shm_ptr->logging); // Lock logging semaphore.
    shm_ptr->avg_rest_time += resttime;
    sem_post(&shm_ptr->logging); // Unlock logging semaphore.

    // Visitor leaves table.
    sem_wait(&shm_ptr->table_sems[found_table]);

    shm_ptr->tables[found_table].chairs[found_chair].occupied_by_pid = 0; // Free chair
    shm_ptr->tables[found_table].num_occupied--;

    if (shm_ptr->tables[found_table].num_occupied == 0) {
        shm_ptr->tables[found_table].blocked = false; // Unblock the table
    }    

    sem_post(&shm_ptr->table_sems[found_table]);

    printf("Visitor %d: Left table %d, chair %d.\n", getpid(), found_table, found_chair);   // Debugging.

    // Unmap shared memory.
    munmap(shm_ptr, SHM_SIZE);  
}
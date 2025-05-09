#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>
#include <semaphore.h>

// Bar Constants
#define NUM_TABLES 3
#define CHAIRS_PER_TABLE 4
#define NUM_MENU_ITEMS 4

typedef struct{
    int occupied_by_pid; // PID of the visitor occupying the chair (0 = unoccupied).
} Chair;

typedef struct{
    bool blocked;
    int num_occupied;
    Chair chairs[CHAIRS_PER_TABLE]; // Array of Chairs.
} Table;

typedef struct {
    Table tables[NUM_TABLES];  // Array of tables.
    int total_visitors;        // Total number of visitors so far.
    int product_stats[NUM_MENU_ITEMS]; // Total consumption of each menu item.
    float avg_rest_time;       // Average time visitors stayed at the bar(resttime).
    float avg_wait_time;
    int order_count;
    sem_t logging;                 // Semaphore for logging.
    sem_t table_sems[NUM_TABLES];  // Array of semaphores for table-level synchronization.
    sem_t wakeup;                  // Semaphore to wake up receptionist.
} SharedMemory;

typedef enum {
    WATER = 0,
    WINE = 1,
    CHEESE = 2,
    SALAD = 3
} MenuItem;

typedef struct {
    int orders[NUM_MENU_ITEMS];
} VisitorOrder;
#endif
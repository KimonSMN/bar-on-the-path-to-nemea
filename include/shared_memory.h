#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>
#include <semaphore.h>

// sem_t mutex;

// Bar Constants
#define NUM_TABLES 1
#define CHAIRS_PER_TABLE 2

typedef struct{
    int occupied_by_pid; // PID of the visitor occupying the chair (0 = unoccupied)
} Chair;

typedef struct{
    // bool is_active;
    int num_occupied;
    Chair chairs[CHAIRS_PER_TABLE]; // array of Chairs
} Table;

typedef struct {
    Table tables[NUM_TABLES];  // Array of tables
    int total_visitors;        // Total number of visitors so far
} SharedMemory;

// enum Food {
    
// };

// enum Drink{

// };
#endif
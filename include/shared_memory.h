#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <stdbool.h>

// Bar Constants
#define NUM_TABLES 3
#define CHAIRS_PER_TABLE 4


typedef struct{
    bool is_active;
    int num_occupied;
    Chair charis[CHAIRS_PER_TABLE]; // array of Chairs
} Table;

typedef struct{
    int occupied; //maybe change it int occupied_by_pid; // Process ID of the visitor occupying the chair (0 if unoccupied)
} Chair;

typedef struct {
    Table tables[NUM_TABLES];  // Array of tables
    int total_visitors;        // Total number of visitors so far
} SharedMemory;

#endif
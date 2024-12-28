#ifndef MONITOR_H
#define MONITOR_H

#include <sys/types.h>
#include "shared_memory.h"

// Struct to represent the Monitor
typedef struct {
    pid_t pid;  // Process ID of the monitor
} Monitor;

// Function to print the status of tables in the bar
void print_table_status(BarSharedMemory* shm);

// Function to print consumption statistics for drinks and food
void print_product_stats(BarSharedMemory* shm);

// Function to display the entire bar status
void display_bar_status(BarSharedMemory* shm);

#endif // MONITOR_H

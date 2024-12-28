#ifndef VISITOR_H
#define VISITOR_H

#include <sys/types.h>
#include "shared_memory.h"

// Struct to represent a Visitor
typedef struct {
    int max_rest_time;  // Maximum rest time for the visitor
    pid_t pid;          // Process ID of the visitor
} Visitor;

// Function to find an available chair for the visitor
void find_available_chair(BarSharedMemory* shm, pid_t pid);

// Function for the visitor to leave the table
void leave_table(BarSharedMemory* shm, pid_t pid);

// Function that simulates the visitor's bar visit, including resting and leaving
void visit_bar(BarSharedMemory* shm, int max_rest_time, pid_t pid);

#endif // VISITOR_H

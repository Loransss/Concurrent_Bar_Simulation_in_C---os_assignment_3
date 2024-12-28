#ifndef RECEPTIONIST_H
#define RECEPTIONIST_H

#include <sys/types.h>
#include "shared_memory.h"

// Struct to represent the Receptionist
typedef struct {
    pid_t pid;  // Process ID of the receptionist
} Receptionist;

// Processes an order for a visitor
void process_order(BarSharedMemory* shm, pid_t visitor_pid);

// Serves a visitor from the shared memory bar
void serve_visitor(BarSharedMemory* shm, int max_order_time);

#endif // RECEPTIONIST_H

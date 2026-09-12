#ifndef RECEPTIONIST_H
#define RECEPTIONIST_H

#include <sys/types.h>
#include "shared_memory.h"

// Processes an order for a visitor
void order(pid_t visitor_pid, Bar* shm);

//Function to update tehe species orders
void update_orders(Bar* shm, int water, int wine, int cheese, int salad);

// Function to find a visitor that needs service
pid_t find_visitor(Bar* shm);

// Serves a visitor from the shared memory bar
void serve_visitor(Bar* shm, int max_order_time);

#endif // RECEPTIONIST_H

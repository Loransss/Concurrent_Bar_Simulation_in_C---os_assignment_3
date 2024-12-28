#pragma once

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>
#include <semaphore.h>

#define MAX_TABLES 3
#define CHAIRS_PER_TABLE 4
#define MAX_VISITORS 12  // Total possible visitors

// Structure to track table status
typedef struct {
    pid_t visitors[CHAIRS_PER_TABLE];  // PIDs of visitors at this table
    int occupancy;  // Number of current visitors
    int is_full;    // Whether table is completely full
    int has_been_full;    // Add this flag to track if table was ever full
} TableStatus;

// Initialize a single table's status
void init_table_status(TableStatus* table);

// Shared memory structure
typedef struct {
    TableStatus tables[MAX_TABLES];
    
    // Statistics tracking
    int total_visitors;
    int total_water_orders;
    int total_wine_orders;
    int total_cheese_orders;
    int total_salad_orders;
    
    double total_visit_duration;
    double total_wait_time;
    
    // Synchronization semaphores
    sem_t table_mutex;          // Mutex for table access
    sem_t chair_available;      // Semaphore for available chairs
    sem_t receptionist_mutex;   // Mutex for receptionist service
} BarSharedMemory;

// Initialize the shared memory structure
void init_bar_shared_memory(BarSharedMemory* shm);

// Function declarations for shared memory management
int create_shared_memory(key_t key);
BarSharedMemory* attach_shared_memory(int shmid);
void detach_shared_memory(BarSharedMemory* shm);
void destroy_shared_memory(int shmid);

#endif

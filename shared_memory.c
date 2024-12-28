#include "shared_memory.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>

void init_table_status(TableStatus* table) {
    memset(table->visitors, 0, sizeof(pid_t) * CHAIRS_PER_TABLE);
    table->occupancy = 0;
    table->is_full = 0;
}

void init_bar_shared_memory(BarSharedMemory* shm) {
    // Initialize all tables
    for (int i = 0; i < MAX_TABLES; i++) {
        init_table_status(&shm->tables[i]);
    }
    
    // Initialize statistics
    shm->total_visitors = 0;
    shm->total_water_orders = 0;
    shm->total_wine_orders = 0;
    shm->total_cheese_orders = 0;
    shm->total_salad_orders = 0;
    shm->total_visit_duration = 0;
    shm->total_wait_time = 0;
    
    // Initialize semaphores
    sem_init(&shm->table_mutex, 0, 1);  // Mutex for table access
    sem_init(&shm->chair_available, 0, MAX_TABLES * CHAIRS_PER_TABLE);  // Semaphore for available chairs
    sem_init(&shm->receptionist_mutex, 0, 1);  // Mutex for receptionist access
}

int create_shared_memory(key_t key) {
    // Create shared memory segment
    int shmid = shmget(key, sizeof(BarSharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Failed to create shared memory");
        return -1;
    }
    
    // Attach to the shared memory segment
    BarSharedMemory* shm = attach_shared_memory(shmid);
    if (shm == NULL) {
        return -1;
    }

    // Initialize shared memory content
    init_bar_shared_memory(shm);
    
    // Detach from the shared memory
    detach_shared_memory(shm);

    return shmid;
}

BarSharedMemory* attach_shared_memory(int shmid) {
    // Attach to the shared memory segment
    void* shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (void*)-1) {
        perror("Failed to attach shared memory");
        return NULL;
    }
    return (BarSharedMemory*) shm_ptr;
}

void detach_shared_memory(BarSharedMemory* shm) {
    // Detach from the shared memory segment
    if (shmdt(shm) == -1) {
        perror("Failed to detach shared memory");
    }
}

void destroy_shared_memory(int shmid) {
    // Attach to the shared memory segment for cleanup
    BarSharedMemory* shm = attach_shared_memory(shmid);
    if (shm == NULL) {
        return;
    }
    
    // Destroy semaphores
    sem_destroy(&shm->table_mutex);
    sem_destroy(&shm->chair_available);
    sem_destroy(&shm->receptionist_mutex);

    // Detach from the shared memory
    detach_shared_memory(shm);
    
    // Remove the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Failed to destroy shared memory");
    }
}

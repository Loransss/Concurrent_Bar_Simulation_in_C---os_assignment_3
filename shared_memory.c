#include "shared_memory.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>

void init_bar_shared_memory(Bar* shm) {
    //Initialize tables
    for (int i = 0; i < TABLES; i++) {
        memset(&shm->tables[i].visitors, 0, sizeof(pid_t) * CHAIRS_PER_TABLE);
        shm->tables[i].occupancy = 0;
        shm->tables[i].is_full = 0;
        shm->tables[i].has_been_full = 0;
    }
    
    //Initialize statistics variables
    shm->visit_duration = 0;
    shm->wait_time = 0;
    
    //Initialize products statistics variables
    shm->total_visitors = 0;
    shm->water_orders = 0;
    shm->wine_orders = 0;
    shm->cheese_orders = 0;
    shm->salad_orders = 0;

    
    //Initialize semaphores
    sem_init(&shm->table_mutex, 0, 1);  //Mutex for table access
    sem_init(&shm->chair_available, 0, TABLES * CHAIRS_PER_TABLE);  //Semaphore for available chairs
    sem_init(&shm->receptionist_mutex, 0, 1);  //Mutex for receptionist access
}


int create_shared_memory(key_t key) {
    //Create shared memory segment
    int shmid = shmget(key, sizeof(Bar), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Failed to create shared memory");
        return -1;
    }
    
    //Attach to the shared memory segment
    Bar* shm = attach_shared_memory(shmid);
    if (shm == NULL) {
        return -1;
    }

    //Initialize shared memory
    init_bar_shared_memory(shm);
    
    //Detach shared memory
    detach_shared_memory(shm);

    return shmid;
}

Bar* attach_shared_memory(int shmid) {
    // Attach to the shared memory segment
    void* shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (void*)-1) {
        perror("Failed to attach shared memory");
        return NULL;
    }
    return (Bar*) shm_ptr;
}

void detach_shared_memory(Bar* shm) {
    //Detach from the shared memory segment
    if (shmdt(shm) == -1) {
        perror("Dettach shared memory failure.\n");
    }
}

void destroy_shared_memory(int shmid) {
    //Attach to the shared memory segment for cleanup
    Bar* shm = attach_shared_memory(shmid);
    if (shm == NULL) {
        return;
    }
    
    //Destroy semaphores
    sem_destroy(&shm->table_mutex);
    sem_destroy(&shm->chair_available);
    sem_destroy(&shm->receptionist_mutex);

    //Detach shared memory
    detach_shared_memory(shm);
    
    //Remove the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Failed to destroy shared memory");
    }
}

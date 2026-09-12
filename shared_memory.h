#pragma once

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>
#include <semaphore.h>

#define MAX_VISITORS 12 
#define TABLES 3
#define CHAIRS_PER_TABLE 4


typedef struct {
    pid_t visitors[CHAIRS_PER_TABLE];  //PIDs of visitors at this table
    int occupancy;  //Number of current visitors
    int is_full;    //Check if the table is full
    int has_been_full; //Check if the table was full
} Table;


typedef struct {
    Table tables[TABLES];
    
    //Process tracking
    pid_t receptionist_pid;
    pid_t visitor_pid[MAX_VISITORS];
    int num_of_visitors;

    //Variables for stats
    double visit_duration;
    double wait_time;

    //Variables for products stats
    int total_visitors;
    int water_orders;
    int wine_orders;
    int cheese_orders;
    int salad_orders;
    
    //Semaphores
    sem_t table_mutex; //Mutex for table access
    sem_t chair_available; //Semaphore for available chairs
    sem_t receptionist_mutex; //Mutex for receptionist service
} Bar;

// Initialize the shared memory structure
void init_bar_shared_memory(Bar* shm);

int create_shared_memory(key_t key);

Bar* attach_shared_memory(int shmid) ;

void detach_shared_memory(Bar* shm);

void destroy_shared_memory(int shmid);

#endif

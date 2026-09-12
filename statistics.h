// statistics.h
#ifndef STATISTICS_H
#define STATISTICS_H

#include "shared_memory.h"

//Parse the command line arguments
void parse_arguments(int argc, char* argv[], key_t* shmkey);

//Function that terminates all processes
void terminate_processes(Bar* shm);

// Calculate and display final statistics
void display_final_statistics(Bar* shm);

// Calculate average duration of stay
double calculate_average_duration(Bar* shm);

// Calculate average waiting time
double calculate_average_wait_time(Bar* shm);

// Display order statistics
void display_order_statistics(Bar* shm);

#endif // STATISTICS_H
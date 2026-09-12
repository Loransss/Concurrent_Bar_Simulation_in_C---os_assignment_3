#ifndef VISITOR_H
#define VISITOR_H

#include <sys/types.h>
#include "shared_memory.h"

//Helper function to find and seat a visitor at a table
int seat_visitor(Bar* shm, pid_t pid, int start_table);

//Finds an available chair for the visitor
void find_chair( pid_t pid, Bar* shm) ;

//Parse the command line arguments
void parse_arguments(int argc, char* argv[], int* rest_time, key_t* shmkey);

//Leaves the table
void leave_table(pid_t pid, Bar* shm);

//Function tha simualtes the bar visit
void bar_visit(Bar* shm, int max_rest_time, pid_t pid);

#endif // VISITOR_H

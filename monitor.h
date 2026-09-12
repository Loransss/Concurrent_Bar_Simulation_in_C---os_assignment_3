#ifndef MONITOR_H
#define MONITOR_H

#include <sys/types.h>
#include "shared_memory.h"

//Fuction that dispalys the bar monitor
void display_bar(Bar* shm);

//Function to parse the arguments from the programm call
void parse_arguments(int argc, char* argv[], key_t* shmkey);

#endif // MONITOR_H

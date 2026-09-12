#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "shared_memory.h"

//Helper function to find and seat a visitor at a table
int seat_visitor(Bar* shm, pid_t pid, int start_table) {
    for (int i=0; i<TABLES; ++i) {
        int current_table = (start_table + i) % TABLES; // Circular iteration

        if (shm->tables[current_table].occupancy < CHAIRS_PER_TABLE && !shm->tables[current_table].has_been_full) {
            //Add visitor to an available chair
            for (int j = 0; j<CHAIRS_PER_TABLE; ++j) {
                if (shm->tables[current_table].visitors[j] == 0) {
                    shm->tables[current_table].visitors[j] = pid;
                    shm->tables[current_table].occupancy++;

                    printf("Visitor %d seated at table %d, chair %d\n", pid, current_table, j);

                    // Update table state if fully occupied
                    if (shm->tables[current_table].occupancy == CHAIRS_PER_TABLE) {
                        shm->tables[current_table].is_full = 1;
                        shm->tables[current_table].has_been_full = 1;
                    }
                    return (current_table + 1) % TABLES; //Return next starting point
                }
            }
        }
    }

    return start_table; //If no chair found, return the same starting point
}

//Find a chair for the visitor
void find_chair(pid_t pid, Bar* shm) {
    static int last_checked_table = 0;

    //Store visitor PID in shared memory for tracking
    sem_wait(&shm->receptionist_mutex);
    if (shm->num_of_visitors < MAX_VISITORS) {
        shm->visitor_pid[shm->num_of_visitors++] = pid;
    }
    sem_post(&shm->receptionist_mutex);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);  //Start timing the wait

    while (1) {
        sem_wait(&shm->chair_available);
        sem_wait(&shm->table_mutex);

        //Attempt to find and seat the visitor
        int new_start_table = seat_visitor(shm, pid, last_checked_table);

        sem_post(&shm->table_mutex);

        if (new_start_table != last_checked_table) {
            //Visitor was successfully seated
            last_checked_table = new_start_table;
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            double wait_time = (end_time.tv_sec - start_time.tv_sec) + 
                             (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
            
            //Update statistics atomically
            sem_wait(&shm->receptionist_mutex);
            shm->wait_time += wait_time;
            shm->total_visitors++;
            sem_post(&shm->receptionist_mutex);

            last_checked_table = new_start_table;
            sem_post(&shm->table_mutex);
            return;
        }

        sem_post(&shm->table_mutex);
        printf("Visitor %d waiting for available chair...\n", pid);
        sleep(1); //Wait and try again
    }
}

//Leaves the table
void leave_table(pid_t pid, Bar* shm) {
    sem_wait(&shm->table_mutex);

    for (int i=0; i<TABLES; ++i) {
        for (int j=0; j<CHAIRS_PER_TABLE; ++j) {
            if (shm->tables[i].visitors[j] == pid) {
                shm->tables[i].visitors[j] = 0;
                shm->tables[i].occupancy--;
                shm->tables[i].is_full = 0;
                //If the table gets empty, the visitor can sit
                if(shm->tables[i].occupancy == 0){
                    shm->tables[i].has_been_full = 0;
                }
                printf("Visitor %d leaving table %d\n", pid, i);
                break;
            }
        }
    }

    sem_post(&shm->table_mutex);
    sem_post(&shm->chair_available);
}

//Function tha simualtes the bar visit
void bar_visit(Bar* shm, int max_rest_time, pid_t pid) {
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    //Find an available chair
    find_chair(pid, shm);

    //Find the rest time
    srand(time(NULL));
    double min_rest_time = 0.7 * max_rest_time; // Minimum rest time
    double rest_time = min_rest_time + ((double)rand() / RAND_MAX) * (max_rest_time - min_rest_time);
    printf("Visitor %d will stay for %f seconds\n", pid, rest_time);

    sleep(rest_time);

    //Leave table
    leave_table(pid, shm);

    //Calculate and update visit duration
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double duration = (end_time.tv_sec - start_time.tv_sec) + 
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    sem_wait(&shm->receptionist_mutex);
    shm->visit_duration += duration;
    sem_post(&shm->receptionist_mutex);
}

//Parse the command line arguments
void parse_arguments(int argc, char* argv[], int* rest_time, key_t* shmkey) {
    if (argc < 2) {
        printf("Using default settings: order_time=%d, shmkey=%d\n", *rest_time, *shmkey);
        return;
    }

    //Try to read key from file first
    FILE *key_file = fopen("bar_key.txt", "r");
    if (key_file) {
        fscanf(key_file, "%d", shmkey);
        fclose(key_file);
    }

    for (int i=1; i<argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 && (i + 1) < argc) {
            *rest_time = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && (i + 1) < argc) {
            *shmkey = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Invalid argument passing.\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char* argv[]) {
    int rest_time = 15;  //Default rest time 
    key_t shmkey;

    parse_arguments(argc, argv, &rest_time, &shmkey);

    printf("\nVisitor starting with rest time: %d seconds\n", rest_time);

    //Attach the shared memory

    //Get shared memory segment
    int shmid = shmget(shmkey, sizeof(Bar), 0666);
    if (shmid == -1) {
        fprintf(stderr, "Get shared memory segment failure!\n");
        return 1;
    }

    //Attach shared memory
    Bar* shm = attach_shared_memory(shmid);

    pid_t pid = getpid();
    bar_visit(shm, rest_time, pid);

    //Detach the shared memory
    detach_shared_memory(shm);

    return 0;
}

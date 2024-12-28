#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_memory.h"

// Finds an available chair for the visitor
void find_available_chair(BarSharedMemory* shm, pid_t pid) {
    printf("Visitor %d looking for a chair...\n", pid);

    while (true) {
        sem_wait(&shm->chair_available);
        sem_wait(&shm->table_mutex);

        bool found = false;

        // Find a table with space
        for (int i = 0; i < MAX_TABLES; ++i) {
            if (shm->tables[i].occupancy < CHAIRS_PER_TABLE) {
                // Add visitor to table
                for (int j = 0; j < CHAIRS_PER_TABLE; ++j) {
                    if (shm->tables[i].visitors[j] == 0) {
                        shm->tables[i].visitors[j] = pid;
                        shm->tables[i].occupancy++;

                        printf("Visitor %d sat at table %d, chair %d\n", pid, i, j);

                        if (shm->tables[i].occupancy == CHAIRS_PER_TABLE) {
                            shm->tables[i].is_full = true;
                        }

                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
        }

        sem_post(&shm->table_mutex);
        if (found) return;

        printf("Visitor %d waiting for available chair...\n", pid);
        sleep(1); // Wait and retry
    }
}

// Leaves the table after resting
void leave_table(BarSharedMemory* shm, pid_t pid) {
    sem_wait(&shm->table_mutex);

    for (int i = 0; i < MAX_TABLES; ++i) {
        for (int j = 0; j < CHAIRS_PER_TABLE; ++j) {
            if (shm->tables[i].visitors[j] == pid) {
                shm->tables[i].visitors[j] = 0;
                shm->tables[i].occupancy--;
                shm->tables[i].is_full = false;

                printf("Visitor %d leaving table %d, chair %d\n", pid, i, j);
                break;
            }
        }
    }

    sem_post(&shm->table_mutex);
    sem_post(&shm->chair_available);
}

// Simulates a visit to the bar
void visit_bar(BarSharedMemory* shm, int max_rest_time, pid_t pid) {
    // Find an available chair
    find_available_chair(shm, pid);

    // Rest at the table
    int rest_duration = (rand() % (max_rest_time / 2)) + (max_rest_time / 2);
    printf("Visitor %d will stay for %d seconds\n", pid, rest_duration);

    sleep(rest_duration);

    // Leave the table
    leave_table(shm, pid);
}

int main(int argc, char* argv[]) {
    int opt;
    int rest_time = 15;  // Default rest time increased to 15 seconds
    key_t shmkey = 1108402178; // Default shared memory key

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "d:s:")) != -1) {
        switch (opt) {
            case 'd':
                rest_time = atoi(optarg);
                break;
            case 's':
                shmkey = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-d rest_time] [-s shmkey]\n", argv[0]);
                return 1;
        }
    }

    printf("Visitor process starting with rest time: %d seconds\n", rest_time);

    // Attach to existing shared memory
    int shmid = shmget(shmkey, sizeof(BarSharedMemory), 0666);
    if (shmid == -1) {
        perror("Failed to get shared memory");
        return 1;
    }

    BarSharedMemory* shm = (BarSharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (void*)-1) {
        perror("Failed to attach shared memory");
        return 1;
    }

    pid_t pid = getpid();
    visit_bar(shm, rest_time, pid);

    // Detach shared memory
    if (shmdt(shm) == -1) {
        perror("Failed to detach shared memory");
        return 1;
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "shared_memory.h"

// Processes an order for a visitor
void process_order(BarSharedMemory* shm, pid_t visitor_pid) {
    // Randomly decide on drinks and food
    bool water = rand() % 2;
    bool wine = rand() % 2;
    bool cheese = rand() % 2;
    bool salad = rand() % 2;

    // At least one drink is mandatory
    if (!water && !wine) {
        water = true;
    }

    printf("Processing order for visitor %d\n", visitor_pid);

    sem_wait(&shm->receptionist_mutex);

    // Update drink orders
    if (water) {
        shm->total_water_orders++;
        printf("Water ordered\n");
    }
    if (wine) {
        shm->total_wine_orders++;
        printf("Wine ordered\n");
    }

    // Update food orders
    if (cheese) {
        shm->total_cheese_orders++;
        printf("Cheese ordered\n");
    }
    if (salad) {
        shm->total_salad_orders++;
        printf("Salad ordered\n");
    }

    sem_post(&shm->receptionist_mutex);
}

// Serves a visitor from the shared memory bar
void serve_visitor(BarSharedMemory* shm, int max_order_time) {
    // Find a visitor that needs service
    bool visitor_found = false;
    pid_t visitor_pid = 0;

    sem_wait(&shm->table_mutex);
    
    // Check all tables for visitors
    for (int i = 0; i < MAX_TABLES; ++i) {
        for (int j = 0; j < CHAIRS_PER_TABLE; ++j) {
            if (shm->tables[i].visitors[j] != 0) {
                visitor_pid = shm->tables[i].visitors[j];
                visitor_found = true;
                break;
            }
        }
        if (visitor_found) break;
    }
    
    sem_post(&shm->table_mutex);

    if (visitor_found) {
        // Service time is random between 0.5 * max_order_time and max_order_time
        int service_time = (rand() % (max_order_time / 2)) + (max_order_time / 2);

        // Process order for the found visitor
        process_order(shm, visitor_pid);

        // Simulate service time
        sleep(service_time);
    } else {
        // No visitors to serve, wait a bit
        usleep(100000); // Sleep for 0.1 seconds
    }
}

int main(int argc, char* argv[]) {
    int opt;
    int order_time = 10;  // Default order time
    key_t shmkey = 1108402178;  // Default shared memory key

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "d:s:")) != -1) {
        switch (opt) {
            case 'd':
                order_time = atoi(optarg);
                break;
            case 's':
                shmkey = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-d order_time] [-s shmkey]\n", argv[0]);
                return 1;
        }
    }

    printf("Receptionist starting with order time: %d\n", order_time);

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

    srand(time(NULL));

    printf("Receptionist ready to serve visitors\n");

    // Main service loop
    while (1) {
        serve_visitor(shm, order_time);
    }

    // Detach shared memory (not reached in this infinite loop)
    if (shmdt(shm) == -1) {
        perror("Failed to detach shared memory");
        return 1;
    }

    return 0;
}

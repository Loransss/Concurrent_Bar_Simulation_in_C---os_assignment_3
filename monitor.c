#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_memory.h"

// Print table occupancy and visitor details
void print_table_status(BarSharedMemory* shm) {
    for (int i = 0; i < MAX_TABLES; ++i) {
        printf("Table %d Occupancy: %d/%d\n", i, shm->tables[i].occupancy, CHAIRS_PER_TABLE);
        printf("  Visitors: ");
        for (int j = 0; j < CHAIRS_PER_TABLE; ++j) {
            if (shm->tables[i].visitors[j] != 0) {
                printf("%d ", shm->tables[i].visitors[j]);
            }
        }
        printf("\n");
    }
}

// Print consumption statistics
void print_product_stats(BarSharedMemory* shm) {
    printf("\nConsumption Statistics:\n");

    // Print drinks statistics
    printf("Drinks:\n");
    printf("  Water Orders: %d\n", shm->total_water_orders);
    printf("  Wine Orders: %d\n", shm->total_wine_orders);

    // Print food statistics
    printf("Foods:\n");
    printf("  Cheese Orders: %d\n", shm->total_cheese_orders);
    printf("  Salad Orders: %d\n", shm->total_salad_orders);
}

// Display the entire bar status
void display_bar_status(BarSharedMemory* shm) {
    printf("Bar Status Monitor\n");
    printf("-------------------\n");

    print_table_status(shm);
    print_product_stats(shm);
}

int main(int argc, char* argv[]) {
    int opt;
    key_t shmkey = 1108402178;  // Default shared memory key

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "s:")) != -1) {
        switch (opt) {
            case 's':
                shmkey = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-s shmkey]\n", argv[0]);
                return 1;
        }
    }

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

    // Display the bar status
    display_bar_status(shm);

    // Detach shared memory
    if (shmdt(shm) == -1) {
        perror("Failed to detach shared memory");
        return 1;
    }

    return 0;
}

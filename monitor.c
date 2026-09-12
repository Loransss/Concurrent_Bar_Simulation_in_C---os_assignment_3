#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_memory.h"

//Fuction that dispalys the bar monitor
void display_bar(Bar* shm) {
    printf("Monitor of the bar\n\n");

    //Print the situation in the tables
    printf("----- Table situation -----\n");
    for (int i=0; i < TABLES; ++i){
        int table_occupancy =  shm->tables[i].occupancy;
        printf("\nTable %d: %d/%d\n", i, table_occupancy, CHAIRS_PER_TABLE);
        printf("  ~Visitors of Table %d: ", i);
        for (int j=0; j<CHAIRS_PER_TABLE; ++j) {
            int table_visitor = shm->tables[i].visitors[j];
            if (table_visitor != 0) {
                printf("%d ", table_visitor);
            }
        }
        printf("\n");
    }

    //Print the product statistics
    printf("\n-----Product stats-----\n");

    printf("Drinks:\n");
    int water = shm->water_orders;
    int wine = shm->wine_orders;
    printf("Number of water orders: %d\n", water);
    printf("Number of wine orders: %d\n", wine);

    // Print food statistics
    printf("\nFoods:\n");
    int cheese = shm->cheese_orders;
    int salad = shm->salad_orders;
    printf("Number of cheese orders: %d\n", cheese);
    printf("Number of salad orders: %d\n", salad);
}

//Parse the command line arguments
void parse_arguments(int argc, char* argv[], key_t* shmkey) {
    // Try to read key from file first
    FILE *key_file = fopen("bar_key.txt", "r");
    if (key_file) {
        fscanf(key_file, "%d", shmkey);
        fclose(key_file);
    }

    if(argc == 3 && strcmp(argv[1], "-s") == 0) {
        if (sscanf(argv[2], "%d", shmkey) != 1) {
            fprintf(stderr, "Error: Invalid shared memory key format.\n");
            exit(EXIT_FAILURE);
        }
    }else if (argc != 1) {
        fprintf(stderr, "Error in program call.\n");
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char* argv[]) {
    key_t shmkey;

    parse_arguments(argc, argv, &shmkey);

    //Attach the shared memory

    //Get shared memory segment
    int shmid = shmget(shmkey, sizeof(Bar), 0666);
    if (shmid == -1) {
        fprintf(stderr, "Get shared memory segment failure!\n");
        return 1;
    }

    //Attach shared memory
    Bar* shm = attach_shared_memory(shmid);

    //Display the bar status
    display_bar(shm);

    //Detach the shared memory
    detach_shared_memory(shm);

    return 0;
}

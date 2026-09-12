#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <time.h>
#include "shared_memory.h"


//Function to update the species orders
void update_orders(Bar* shm, int water, int wine, int cheese, int salad){
    if(water){
        printf("Visitor ordered water.\n");
        shm->water_orders++;
    }
    if(wine){
        printf("Visitor ordered wine.\n");
        shm->wine_orders++;
    }
    if(cheese){
        printf("Visitor ordered cheese.\n");
        shm->cheese_orders++;
    }
    if(salad){
        printf("Visitor ordered salad.\n");
        shm->salad_orders++;
    }
}

//Order procces
void order(pid_t visitor_pid, Bar* shm) {
    //Randoms chose food and drink
    srand(time(NULL));
    int water = rand() % 2;
    int wine = rand() % 2;
    int cheese = rand() % 2;
    int salad = rand() % 2;

    //everyone has to take at least one drink
    if (!water && !wine) {
        if(rand()%2){
            water = 1;
        }
        else{
            wine = 1;
        }
    }

    sem_wait(&shm->receptionist_mutex);

    //Update the species orders
    update_orders(shm, water, wine, cheese, salad);

    sem_post(&shm->receptionist_mutex);
}

//Function to find a visitor that needs service
pid_t find_visitor(Bar* shm) {
    sem_wait(&shm->table_mutex);

    // Check all tables for visitors
    for (int i=0; i<TABLES; ++i) {
        for (int j=0; j<CHAIRS_PER_TABLE; ++j) {
            int visitor_to_serve = shm->tables[i].visitors[j];
            if (visitor_to_serve != 0) {;
                sem_post(&shm->table_mutex);
                return visitor_to_serve; //Visitor found
            }
        }
    }

    sem_post(&shm->table_mutex);
    return 0; //No visitor found
}

// Function to serve a visitor
void serve_visitor(Bar* shm, int max_order_time) {
    //Find a visitor that needs service
    pid_t visitor_to_serve = find_visitor(shm);

    if(visitor_to_serve != 0){
        //Service time is random between 0.5 * max_order_time and max_order_time
        srand(time(NULL));
        double min_order_time = 0.5 * max_order_time; //Minimum service time
        double service_time = min_order_time + ((double)rand() / RAND_MAX) * (max_order_time - min_order_time);

        //Process order for the found visitor
        order(visitor_to_serve, shm);

        //Sleep for the service time
        sleep(service_time);
    }else {
        //No visitor to serve so we wait
        usleep(100000);
    }
}

//Parse the command line arguments
void parse_arguments(int argc, char* argv[], int* order_time, key_t* shmkey) {
    if (argc < 2) {
        printf("Using default settings: order_time=%d, shmkey=%d\n", *order_time, *shmkey);
        return;
    }

    // Try to read key from file first
    FILE *key_file = fopen("bar_key.txt", "r");
    if (key_file) {
        fscanf(key_file, "%d", shmkey);
        fclose(key_file);
    }

    for (int i=1; i<argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 && (i + 1) < argc) {
            *order_time = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-s") == 0 && (i + 1) < argc) {
            *shmkey = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Invalid argument passing.\n");
            exit(EXIT_FAILURE);
        }
    }
}


int main(int argc, char* argv[]) {
    int order_time = 10;  //Default order time
    key_t shmkey; 

    parse_arguments(argc, argv, &order_time, &shmkey);

    printf("\nReceptionist starting with order time: %d\n", order_time);

    //Attach the shared memory

    //Get shared memory segment
    int shmid = shmget(shmkey, sizeof(Bar), 0666);
    if (shmid == -1) {
        fprintf(stderr, "Get shared memory segment failure!\n");
        return 1;
    }

    //Attach shared memory
    Bar* shm = attach_shared_memory(shmid);

    shm->receptionist_pid = getpid();

    //Serve loop
    while (1){
        serve_visitor(shm, order_time);
    }

    //Detach the shared memory
    detach_shared_memory(shm);

    return 0;
}

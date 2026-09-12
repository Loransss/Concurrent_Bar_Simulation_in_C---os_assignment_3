#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include "statistics.h"
#include "shared_memory.h"

//Functiοn that terminates all processes
void terminate_processes(Bar* shm) {
    printf("\nTerminating processes...\n");
    
    //Terminate visitors first
    for (int i = 0; i < shm->num_of_visitors; i++) {
        if (shm->visitor_pid[i] > 0) {
            printf("Terminating visitor %d\n", shm->visitor_pid[i]);
            kill(shm->visitor_pid[i], SIGTERM);
        }
    }
    
    //Small delay to allow visitors to cleanup
    usleep(100000);  //100ms
    
    //Terminate receptionist
    if (shm->receptionist_pid > 0) {
        printf("Terminating receptionist %d\n", shm->receptionist_pid);
        kill(shm->receptionist_pid, SIGTERM);
    }
    
    printf("All processes terminated.\n");
}

//Function that displays the final statistics
void display_final_statistics(Bar* shm) {
    printf("\n----- Final Bar Statistics -----\n\n");

    //1. Total number of visitors
    printf("1. Total number of visitors: %d\n", shm->total_visitors);

    //2. Average duration of stay
    double avg_duration = calculate_average_duration(shm);
    printf("2. Average duration of stay: %.2f seconds\n", avg_duration);

    //3. Average waiting time
    double avg_wait = calculate_average_wait_time(shm);
    printf("3. Average waiting time for a seat: %.2f seconds\n", avg_wait);

    //4. Order statistics
    display_order_statistics(shm);
}

double calculate_average_duration(Bar* shm) {
    if (shm->total_visitors == 0) return 0.0;
    return shm->visit_duration / shm->total_visitors;
}

double calculate_average_wait_time(Bar* shm) {
    if (shm->total_visitors == 0) return 0.0;
    return shm->wait_time / shm->total_visitors;
}

void display_order_statistics(Bar* shm) {
    printf("\n4. Order Statistics:\n");
    printf("   Drinks:\n");
    printf("   - Water orders: %d\n", shm->water_orders);
    printf("   - Wine orders: %d\n", shm->wine_orders);
    printf("   Food:\n");
    printf("   - Cheese orders: %d\n", shm->cheese_orders);
    printf("   - Salad orders: %d\n", shm->salad_orders);
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

//Main program for statistics and cleanup
int main(int argc, char* argv[]) {
    key_t shmkey;

    parse_arguments(argc, argv, &shmkey);

    // Get shared memory segment
    int shmid = shmget(shmkey, sizeof(Bar), 0666);
    if (shmid == -1) {
        fprintf(stderr, "Failed to get shared memory segment!\n");
        return 1;
    }

    // Attach shared memory
    Bar* shm = attach_shared_memory(shmid);

    // Terminate processes
    terminate_processes(shm);
    
    // Wait briefly for processes to cleanup
    sleep(1);


    // Display final statistics
    display_final_statistics(shm);

    // Cleanup: Destroy semaphores and shared memory
    destroy_shared_memory(shmid);
    printf("\nShared memory and semaphores cleaned up successfully.\n");

    return 0;
}
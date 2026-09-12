#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include "shared_memory.h"

//Create and initialize the shared memory
int main() {
    
    //Create the unique key
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        fprintf(stderr, "ftok failure.\n");
        return 1;
    }
    
    // Save the key to a file
    FILE *key_file = fopen("bar_key.txt", "w");
    if (key_file) {
        fprintf(key_file, "%d", key);
        fclose(key_file);
    }

    //Create the shared memory
    int shmid = create_shared_memory(key);
    if (shmid == -1) {
        fprintf(stderr, "shared memory create failure.\n");
        return 1;
    }

    printf("Key: %d\n", key);
    printf("ID: %d\n", shmid);
    
    return 0;
}

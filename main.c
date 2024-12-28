#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include "shared_memory.h"


int main() {
    key_t key = ftok("/tmp", 'B');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }
    
    int shmid = create_shared_memory(key);
    if (shmid == -1) {
        return 1;
    }
    
    printf("Shared memory initialized\n");
    printf("Key: %d\n", key);
    printf("ID: %d\n", shmid);
    
    return 0;
}

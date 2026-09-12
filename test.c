#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#define NUM_VISITORS 10
#define VISITOR_REST_TIME 15
#define RECEPTIONIST_ORDER_TIME 10

//Function to execute a program with arguments
void execute_program(const char* program, char* const argv[]) {
    execvp(program, argv);
    //If execvp returns, there was an error
    perror("execvp failed");
    exit(1);
}

//Function to create and run a process
pid_t run_process(const char* program, char* const argv[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }
    
    if (pid == 0) {  //Child process
        execute_program(program, argv);
    }
    
    return pid;  //Return pid to parent
}

int main() {
    pid_t main_pid, receptionist_pid;
    pid_t visitor_pids[NUM_VISITORS];
    pid_t monitor_pid;
    
    printf("Starting bar simulation test...\n\n");
    
    //Start main program to create shared memory
    char* main_args[] = {"./main", NULL};
    main_pid = run_process("./main", main_args);
    printf("Started main process (PID: %d)\n", main_pid);
    
    //Wait for main to finish
    waitpid(main_pid, NULL, 0);
    sleep(1);  //Give time for shared memory to be properly set up
    
    //Get shared memory key from file
    FILE* key_file = fopen("bar_key.txt", "r");
    if (!key_file) {
        fprintf(stderr, "Could not read shared memory key\n");
        return 1;
    }
    char shmkey_str[32];
    fgets(shmkey_str, sizeof(shmkey_str), key_file);
    fclose(key_file);

    //Remove newline if present
    shmkey_str[strcspn(shmkey_str, "\n")] = 0;
    
    char receptionist_order_time[12];
    sprintf(receptionist_order_time, "%d", RECEPTIONIST_ORDER_TIME);
    //Start receptionist
    char* receptionist_args[] = {
        "./receptionist",
        "-d", receptionist_order_time,
        "-s", shmkey_str,
        NULL
    };
    receptionist_pid = run_process("./receptionist", receptionist_args);
    printf("Started receptionist (PID: %d)\n", receptionist_pid);
    sleep(1);  //Give receptionist time to initialize
    
    //Start monitor periodically
    char* monitor_args[] = {
        "./monitor",
        "-s", shmkey_str,
        NULL
    };
    
    //Start visitors
    printf("\nStarting %d visitors...\n", NUM_VISITORS);
    for (int i = 0; i < NUM_VISITORS; i++) {
        char rest_time_str[8];
        sprintf(rest_time_str, "%d", VISITOR_REST_TIME);
        
        char* visitor_args[] = {
            "./visitor",
            "-d", rest_time_str,
            "-s", shmkey_str,
            NULL
        };
        
        visitor_pids[i] = run_process("./visitor", visitor_args);
        printf("Started visitor %d (PID: %d)\n", i+1, visitor_pids[i]);
        
        //Run monitor after every 2 visitors
        if (i % 2 == 1) {
            monitor_pid = run_process("./monitor", monitor_args);
            printf("\nRunning monitor...\n\n");
            waitpid(monitor_pid, NULL, 0);  // Wait for monitor to finish
        }
        
        sleep(2);  //Space out visitor creation
    }
    
    //Wait for all visitors to finish
    printf("\nWaiting for visitors to finish...\n");
    for (int i = 0; i < NUM_VISITORS; i++) {
        waitpid(visitor_pids[i], NULL, 0);
        printf("Visitor %d (PID: %d) finished\n", i+1, visitor_pids[i]);
    }
    
    //Run final monitor
    monitor_pid = run_process("./monitor", monitor_args);
    printf("\nRunning final monitor...\n");
    waitpid(monitor_pid, NULL, 0);
    
    //Run statistics to clean up
    printf("\nRunning statistics and cleanup...\n");
    char* stats_args[] = {
        "./statistics",
        "-s", shmkey_str,
        NULL
    };
    pid_t stats_pid = run_process("./statistics", stats_args);
    waitpid(stats_pid, NULL, 0);
    
    printf("\nTest completed successfully!\n");
    return 0;
}
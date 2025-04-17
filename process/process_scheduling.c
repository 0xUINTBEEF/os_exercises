/**
 * @file process_scheduling.c
 * @brief Demonstration of process scheduling concepts
 * 
 * This program demonstrates different process scheduling concepts
 * including priority scheduling and round-robin scheduling.
 * It uses fork() to create child processes and set their priorities.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <time.h>

#define NUM_PROCESSES 3
#define TIME_SLICE 100000 // 100ms in microseconds

/**
 * @brief Structure to hold process information
 */
typedef struct {
    pid_t pid;
    int priority;
    int burst_time;
    int remaining_time;
} process_t;

process_t processes[NUM_PROCESSES];

/**
 * @brief Set process priority using nice value
 * 
 * @param pid Process ID
 * @param priority Nice value (-20 to 19)
 * @return int 0 on success, -1 on failure
 */
int set_process_priority(pid_t pid, int priority) {
    if (setpriority(PRIO_PROCESS, pid, priority) == -1) {
        perror("setpriority failed");
        return -1;
    }
    return 0;
}

/**
 * @brief Simulate CPU burst for a process
 * 
 * @param process The process to run
 */
void run_process(process_t* process) {
    printf("Process %d running with priority %d\n", 
           process->pid, process->priority);
    
    // Simulate CPU burst
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = TIME_SLICE * 1000; // Convert to nanoseconds
    
    nanosleep(&ts, NULL);
    
    process->remaining_time -= TIME_SLICE;
    if (process->remaining_time <= 0) {
        printf("Process %d completed\n", process->pid);
    }
}

/**
 * @brief Round-robin scheduler implementation
 */
void round_robin_scheduler(void) {
    printf("\nRound-Robin Scheduling:\n");
    
    while (1) {
        int all_completed = 1;
        
        for (int i = 0; i < NUM_PROCESSES; i++) {
            if (processes[i].remaining_time > 0) {
                all_completed = 0;
                run_process(&processes[i]);
            }
        }
        
        if (all_completed) break;
    }
}

/**
 * @brief Priority scheduler implementation
 */
void priority_scheduler(void) {
    printf("\nPriority Scheduling:\n");
    
    // Sort processes by priority (higher priority first)
    for (int i = 0; i < NUM_PROCESSES - 1; i++) {
        for (int j = 0; j < NUM_PROCESSES - i - 1; j++) {
            if (processes[j].priority < processes[j + 1].priority) {
                process_t temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
    
    // Run processes in priority order
    for (int i = 0; i < NUM_PROCESSES; i++) {
        while (processes[i].remaining_time > 0) {
            run_process(&processes[i]);
        }
    }
}

/**
 * @brief Main function demonstrating process scheduling
 */
int main(void) {
    // Initialize processes
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        else if (pid == 0) {
            // Child process
            processes[i].pid = getpid();
            processes[i].priority = 10 - i; // Higher number = lower priority
            processes[i].burst_time = (i + 1) * TIME_SLICE * 2;
            processes[i].remaining_time = processes[i].burst_time;
            
            // Set process priority
            set_process_priority(getpid(), processes[i].priority);
            
            // Child processes exit after initialization
            exit(0);
        }
        else {
            // Parent process
            processes[i].pid = pid;
            processes[i].priority = 10 - i;
            processes[i].burst_time = (i + 1) * TIME_SLICE * 2;
            processes[i].remaining_time = processes[i].burst_time;
        }
    }
    
    // Wait for all child processes to initialize
    sleep(1);
    
    // Run different scheduling algorithms
    round_robin_scheduler();
    priority_scheduler();
    
    return 0;
} 
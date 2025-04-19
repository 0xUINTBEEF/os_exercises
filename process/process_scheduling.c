/**
 * Process Scheduling Demonstration
 * 
 * This program demonstrates different process scheduling concepts
 * including priority scheduling and round-robin scheduling.
 * It uses fork() to create child processes and set their priorities.
 * 
 * Features:
 * - Priority-based scheduling
 * - Round-robin scheduling
 * - Process priority management
 * - Error handling
 * - Resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

// Constants
#define NUM_PROCESSES 3
#define TIME_SLICE 100000  // 100ms in microseconds
#define MAX_PRIORITY 19
#define MIN_PRIORITY -20

// Process information structure
typedef struct {
    pid_t pid;
    int priority;
    int burst_time;
    int remaining_time;
    char name[32];
} process_t;

// Global variables
static process_t processes[NUM_PROCESSES];
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Set process priority using nice value
static int set_process_priority(pid_t pid, int priority) {
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        fprintf(stderr, "Invalid priority value: %d\n", priority);
        return -1;
    }

    if (setpriority(PRIO_PROCESS, pid, priority) == -1) {
        perror("setpriority failed");
        return -1;
    }
    return 0;
}

// Simulate CPU burst for a process
static void run_process(process_t* process) {
    printf("Process %s (PID: %d) running with priority %d\n", 
           process->name, process->pid, process->priority);
    
    // Simulate CPU burst
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = TIME_SLICE * 1000;  // Convert to nanoseconds
    
    nanosleep(&ts, NULL);
    
    process->remaining_time -= TIME_SLICE;
    if (process->remaining_time <= 0) {
        printf("Process %s completed\n", process->name);
    }
}

// Round-robin scheduler implementation
static void round_robin_scheduler(void) {
    printf("\nRound-Robin Scheduling:\n");
    
    while (running) {
        int all_completed = 1;
        
        for (int i = 0; i < NUM_PROCESSES && running; i++) {
            if (processes[i].remaining_time > 0) {
                all_completed = 0;
                run_process(&processes[i]);
            }
        }
        
        if (all_completed) break;
    }
}

// Priority scheduler implementation
static void priority_scheduler(void) {
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
    for (int i = 0; i < NUM_PROCESSES && running; i++) {
        while (processes[i].remaining_time > 0 && running) {
            run_process(&processes[i]);
        }
    }
}

int main(void) {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize processes
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            return EXIT_FAILURE;
        }
        else if (pid == 0) {
            // Child process
            processes[i].pid = getpid();
            processes[i].priority = MAX_PRIORITY - i;  // Higher number = lower priority
            processes[i].burst_time = (i + 1) * TIME_SLICE * 2;
            processes[i].remaining_time = processes[i].burst_time;
            snprintf(processes[i].name, sizeof(processes[i].name), "Process%d", i);
            
            // Set process priority
            if (set_process_priority(getpid(), processes[i].priority) == -1) {
                return EXIT_FAILURE;
            }
            
            // Child processes exit after initialization
            return EXIT_SUCCESS;
        }
        else {
            // Parent process
            processes[i].pid = pid;
            processes[i].priority = MAX_PRIORITY - i;
            processes[i].burst_time = (i + 1) * TIME_SLICE * 2;
            processes[i].remaining_time = processes[i].burst_time;
            snprintf(processes[i].name, sizeof(processes[i].name), "Process%d", i);
        }
    }
    
    // Wait for all child processes to initialize
    sleep(1);
    
    // Run different scheduling algorithms
    round_robin_scheduler();
    priority_scheduler();
    
    // Wait for all child processes to complete
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int status;
        waitpid(processes[i].pid, &status, 0);
    }
    
    return EXIT_SUCCESS;
} 
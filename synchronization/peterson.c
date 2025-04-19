/**
 * Peterson's Algorithm Implementation
 * 
 * This program demonstrates Peterson's algorithm for mutual exclusion
 * between two processes. It provides a software-based solution to the
 * critical section problem without requiring special hardware support.
 * 
 * Features:
 * - Mutual exclusion
 * - Progress
 * - Bounded waiting
 * - Error handling
 * - Graceful shutdown
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

// Constants
#define NUM_PROCESSES 2
#define CRITICAL_SECTION_TIME 2
#define NON_CRITICAL_SECTION_TIME 3
#define MAX_ITERATIONS 5

// Peterson's algorithm variables
static int flag[NUM_PROCESSES] = {0, 0};
static int turn = 0;
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Enter critical section using Peterson's algorithm
static void enter_critical_section(int process_id) {
    int other = 1 - process_id;
    
    // Set flag to indicate interest
    flag[process_id] = 1;
    
    // Give turn to other process
    turn = other;
    
    // Wait while other process is in critical section
    while (flag[other] && turn == other && running) {
        // Busy wait
    }
}

// Exit critical section
static void exit_critical_section(int process_id) {
    flag[process_id] = 0;
}

// Process thread function
static void *process_thread(void *arg) {
    int process_id = *(int *)arg;
    free(arg);
    int iterations = 0;
    
    printf("Process %d starting\n", process_id);
    
    while (running && iterations < MAX_ITERATIONS) {
        // Non-critical section
        printf("Process %d in non-critical section\n", process_id);
        sleep(NON_CRITICAL_SECTION_TIME);
        
        // Enter critical section
        enter_critical_section(process_id);
        if (!running) break;
        
        // Critical section
        printf("Process %d entered critical section\n", process_id);
        sleep(CRITICAL_SECTION_TIME);
        printf("Process %d completed critical section\n", process_id);
        
        // Exit critical section
        exit_critical_section(process_id);
        iterations++;
    }
    
    printf("Process %d completed all iterations\n", process_id);
    return NULL;
}

int main(void) {
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create process threads
    pthread_t threads[NUM_PROCESSES];
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int *process_id = malloc(sizeof(int));
        if (!process_id) {
            perror("malloc failed");
            return EXIT_FAILURE;
        }
        *process_id = i;
        
        if (pthread_create(&threads[i], NULL, process_thread, process_id) != 0) {
            perror("pthread_create failed");
            free(process_id);
            return EXIT_FAILURE;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join failed");
        }
    }
    
    printf("All processes have completed their iterations\n");
    return EXIT_SUCCESS;
}
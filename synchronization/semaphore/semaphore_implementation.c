/**
 * Custom Semaphore Implementation
 * 
 * This program demonstrates a custom implementation of counting semaphores
 * using POSIX signals for process synchronization. The implementation
 * includes proper process management, signal handling, and memory management.
 * 
 * Features:
 * - Counting semaphore implementation
 * - Process synchronization
 * - Signal-based wakeup mechanism
 * - Error handling
 * - Resource cleanup
 * - Performance monitoring
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Constants
#define MAX_WAITING_PROCESSES 100
#define INITIAL_SEM_VALUE 2
#define MAX_PROCESSES 3
#define WORK_TIME 2
#define MAX_ITERATIONS 5

/**
 * @brief Custom semaphore structure
 * 
 * Implements a counting semaphore with a waiting list of processes.
 * Uses volatile qualifiers for variables that may be modified by signal handlers.
 */
typedef struct {
    volatile int val;           /**< Current value of the semaphore */
    volatile int waiting_count; /**< Number of processes currently waiting */
    pid_t* list;               /**< Array of waiting process IDs */
    size_t list_capacity;      /**< Maximum capacity of the waiting list */
    struct timespec start_time; /**< Start time for performance monitoring */
    unsigned long total_operations; /**< Total number of operations */
    unsigned long total_wait_time; /**< Total wait time in nanoseconds */
} my_sem_t;

/** Global semaphore instance for the example */
static my_sem_t* global_sem = NULL;
static volatile sig_atomic_t running = 1;

/**
 * @brief Signal handler for graceful shutdown
 * 
 * Handles SIGINT and SIGTERM signals used to gracefully shut down the program.
 * 
 * @param sig The signal number received
 */
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * @brief Signal handler for waking up waiting processes
 * 
 * Handles SIGUSR1 signals used to wake up processes that were
 * waiting on the semaphore.
 * 
 * @param signo The signal number received
 */
static void wake_handler(int signo) {
    if (signo == SIGUSR1) {
        // Just wake up from pause()
        return;
    }
}

/**
 * @brief Initialize a new semaphore
 * 
 * Allocates and initializes a new semaphore structure with the given
 * initial value. Also allocates the waiting list.
 * 
 * @param initial_value The starting value for the semaphore
 * @return my_sem_t* Pointer to the new semaphore, or NULL on error
 */
my_sem_t* custom_sem_init(int initial_value) {
    my_sem_t* sem = malloc(sizeof(my_sem_t));
    if (!sem) {
        perror("Failed to allocate semaphore");
        return NULL;
    }

    sem->list = malloc(MAX_WAITING_PROCESSES * sizeof(pid_t));
    if (!sem->list) {
        perror("Failed to allocate waiting list");
        free(sem);
        return NULL;
    }

    sem->val = initial_value;
    sem->waiting_count = 0;
    sem->list_capacity = MAX_WAITING_PROCESSES;
    sem->total_operations = 0;
    sem->total_wait_time = 0;
    clock_gettime(CLOCK_MONOTONIC, &sem->start_time);

    return sem;
}

/**
 * @brief Clean up a semaphore
 * 
 * Frees all memory associated with the semaphore.
 * 
 * @param sem Pointer to the semaphore to destroy
 */
void custom_sem_destroy(my_sem_t* sem) {
    if (sem) {
        free(sem->list);
        free(sem);
    }
}

/**
 * @brief Perform the wait (P) operation on a semaphore
 * 
 * Decrements the semaphore value and blocks if it becomes negative.
 * The calling process is added to the waiting list if it needs to block.
 * 
 * @param sem Pointer to the semaphore
 * @return bool true on success, false on error
 */
bool custom_wait(my_sem_t* sem) {
    if (!sem) {
        fprintf(stderr, "Invalid semaphore\n");
        return false;
    }

    struct timespec wait_start, wait_end;
    clock_gettime(CLOCK_MONOTONIC, &wait_start);

    sem->val--;

    if (sem->val < 0) {
        if (sem->waiting_count >= sem->list_capacity) {
            fprintf(stderr, "Too many waiting processes\n");
            sem->val++;
            return false;
        }

        pid_t calling_proc = getpid();
        sem->list[sem->waiting_count++] = calling_proc;

        // Wait for signal
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        
        // Atomically unblock SIGUSR1 and wait for it
        sigprocmask(SIG_UNBLOCK, &mask, NULL);
        pause();
        sigprocmask(SIG_BLOCK, &mask, NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &wait_end);
    sem->total_wait_time += (wait_end.tv_sec - wait_start.tv_sec) * 1000000000 +
                           (wait_end.tv_nsec - wait_start.tv_nsec);
    sem->total_operations++;

    return true;
}

/**
 * @brief Perform the signal (V) operation on a semaphore
 * 
 * Increments the semaphore value and wakes up a waiting process if any.
 * 
 * @param sem Pointer to the semaphore
 * @return bool true on success, false on error
 */
bool custom_signal(my_sem_t* sem) {
    if (!sem) {
        fprintf(stderr, "Invalid semaphore\n");
        return false;
    }

    sem->val++;

    if (sem->val <= 0 && sem->waiting_count > 0) {
        sem->waiting_count--;
        pid_t waken_proc = sem->list[sem->waiting_count];

        if (kill(waken_proc, SIGUSR1) == -1) {
            perror("Failed to wake process");
            return false;
        }
    }

    sem->total_operations++;
    return true;
}

/**
 * @brief Print semaphore statistics
 * 
 * Calculates and prints various statistics about the semaphore's usage.
 * 
 * @param sem Pointer to the semaphore
 */
void print_semaphore_stats(my_sem_t* sem) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    double total_time = (end_time.tv_sec - sem->start_time.tv_sec) +
                       (end_time.tv_nsec - sem->start_time.tv_nsec) / 1e9;
    
    printf("\nSemaphore Statistics:\n");
    printf("Total runtime: %.6f seconds\n", total_time);
    printf("Total operations: %lu\n", sem->total_operations);
    printf("Total wait time: %.6f seconds\n", sem->total_wait_time / 1e9);
    printf("Average wait time: %.6f seconds\n",
           (sem->total_wait_time / 1e9) / sem->total_operations);
    printf("Operations per second: %.2f\n",
           sem->total_operations / total_time);
}

/**
 * @brief Process function
 * 
 * Represents a single process's interaction with the semaphore.
 * 
 * @param process_id The ID of the process
 */
void process_function(int process_id) {
    int iterations = 0;
    
    while (running && iterations < MAX_ITERATIONS) {
        printf("Process %d (PID: %d) trying to acquire semaphore\n",
               process_id, getpid());
        
        if (custom_wait(global_sem)) {
            printf("Process %d (PID: %d) acquired semaphore (value: %d)\n",
                   process_id, getpid(), global_sem->val);
            
            // Simulate some work
            sleep(WORK_TIME);
            
            if (custom_signal(global_sem)) {
                printf("Process %d (PID: %d) released semaphore (value: %d)\n",
                       process_id, getpid(), global_sem->val);
            }
        }
        
        iterations++;
        sleep(1); // Sleep between iterations
    }
}

/**
 * @brief Main function demonstrating semaphore usage
 * 
 * Creates multiple child processes that compete for the semaphore.
 * Each process attempts to acquire the semaphore, performs some work,
 * and then releases it.
 * 
 * @return int Exit status (0 on success, 1 on error)
 */
int main(void) {
    // Set up signal handlers
    struct sigaction sa;
    sa.sa_handler = wake_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Failed to set up signal handler");
        return EXIT_FAILURE;
    }
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize semaphore
    global_sem = custom_sem_init(INITIAL_SEM_VALUE);
    if (!global_sem) {
        return EXIT_FAILURE;
    }

    printf("Initial semaphore value is %d\n", global_sem->val);

    // Create child processes
    pid_t pids[MAX_PROCESSES];
    for (int i = 0; i < MAX_PROCESSES; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("Fork failed");
            custom_sem_destroy(global_sem);
            return EXIT_FAILURE;
        }
        
        if (pids[i] == 0) { // Child process
            process_function(i);
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all child processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("Waitpid failed");
        }
    }

    // Print statistics
    print_semaphore_stats(global_sem);

    // Clean up
    custom_sem_destroy(global_sem);
    return EXIT_SUCCESS;
}
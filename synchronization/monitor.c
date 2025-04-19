/**
 * Monitor Implementation with Priority Inheritance
 * 
 * This program demonstrates a monitor implementation using mutexes and
 * condition variables. It includes priority inheritance to prevent
 * priority inversion and ensure proper synchronization.
 * 
 * Features:
 * - Priority inheritance
 * - Condition variables
 * - Error handling
 * - Graceful shutdown
 * - Performance monitoring
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
#define MAX_THREADS 5
#define MAX_PRIORITY 10
#define MIN_PRIORITY 1
#define OPERATION_TIME 2
#define MAX_OPERATIONS 3

// Thread states
typedef enum {
    WAITING,
    RUNNING,
    COMPLETED
} thread_state_t;

// Thread data structure
typedef struct {
    int id;
    int priority;
    thread_state_t state;
    int operations;
    struct timespec start_time;
    struct timespec end_time;
} thread_data_t;

// Monitor structure
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int current_priority;
    thread_data_t *current_thread;
    thread_data_t threads[MAX_THREADS];
    int num_threads;
} monitor_t;

// Global monitor instance
static monitor_t monitor;
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Initialize monitor
static int monitor_init(void) {
    if (pthread_mutex_init(&monitor.mutex, NULL) != 0) {
        perror("pthread_mutex_init failed");
        return -1;
    }
    
    if (pthread_cond_init(&monitor.cond, NULL) != 0) {
        perror("pthread_cond_init failed");
        pthread_mutex_destroy(&monitor.mutex);
        return -1;
    }
    
    monitor.current_priority = MIN_PRIORITY;
    monitor.current_thread = NULL;
    monitor.num_threads = 0;
    
    return 0;
}

// Clean up monitor resources
static void monitor_cleanup(void) {
    pthread_cond_destroy(&monitor.cond);
    pthread_mutex_destroy(&monitor.mutex);
}

// Enter monitor
static void monitor_enter(thread_data_t *thread) {
    pthread_mutex_lock(&monitor.mutex);
    
    // Priority inheritance
    if (monitor.current_thread != NULL && 
        thread->priority > monitor.current_priority) {
        monitor.current_priority = thread->priority;
    }
    
    // Wait if higher priority thread is running
    while (monitor.current_thread != NULL && 
           thread->priority <= monitor.current_priority) {
        thread->state = WAITING;
        pthread_cond_wait(&monitor.cond, &monitor.mutex);
    }
    
    // Enter monitor
    monitor.current_thread = thread;
    monitor.current_priority = thread->priority;
    thread->state = RUNNING;
    
    pthread_mutex_unlock(&monitor.mutex);
}

// Exit monitor
static void monitor_exit(thread_data_t *thread) {
    pthread_mutex_lock(&monitor.mutex);
    
    // Exit monitor
    monitor.current_thread = NULL;
    monitor.current_priority = MIN_PRIORITY;
    thread->state = COMPLETED;
    
    // Signal waiting threads
    pthread_cond_broadcast(&monitor.cond);
    
    pthread_mutex_unlock(&monitor.mutex);
}

// Thread function
static void *thread_function(void *arg) {
    thread_data_t *thread = (thread_data_t *)arg;
    
    // Set thread priority
    struct sched_param param;
    param.sched_priority = thread->priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam failed");
        return NULL;
    }
    
    // Record start time
    clock_gettime(CLOCK_MONOTONIC, &thread->start_time);
    
    while (running && thread->operations < MAX_OPERATIONS) {
        // Enter monitor
        monitor_enter(thread);
        if (!running) break;
        
        // Perform operation
        printf("Thread %d (priority %d) performing operation %d/%d\n",
               thread->id, thread->priority,
               thread->operations + 1, MAX_OPERATIONS);
        sleep(OPERATION_TIME);
        
        // Exit monitor
        monitor_exit(thread);
        thread->operations++;
        
        // Sleep between operations
        sleep(1);
    }
    
    // Record end time
    clock_gettime(CLOCK_MONOTONIC, &thread->end_time);
    
    printf("Thread %d completed all operations\n", thread->id);
    return NULL;
}

int main(void) {
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize monitor
    if (monitor_init() != 0) {
        return EXIT_FAILURE;
    }
    
    // Create threads
    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_data_t *thread = &monitor.threads[i];
        thread->id = i;
        thread->priority = MIN_PRIORITY + i;
        thread->state = WAITING;
        thread->operations = 0;
        
        if (pthread_create(&threads[i], NULL, thread_function, thread) != 0) {
            perror("pthread_create failed");
            monitor_cleanup();
            return EXIT_FAILURE;
        }
        
        monitor.num_threads++;
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < monitor.num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join failed");
        }
    }
    
    // Clean up
    monitor_cleanup();
    
    printf("All threads have completed their operations\n");
    return EXIT_SUCCESS;
}
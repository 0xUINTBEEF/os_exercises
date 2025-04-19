/**
 * Test Monitor Implementation
 * 
 * This program tests the monitor implementation with various thread
 * priorities and operations. It verifies the correctness of the
 * priority inheritance mechanism and thread synchronization.
 * 
 * Features:
 * - Priority inheritance testing
 * - Thread synchronization verification
 * - Error handling
 * - Performance monitoring
 * - Test result reporting
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
#define NUM_TEST_THREADS 3
#define TEST_OPERATION_TIME 1
#define TEST_MAX_OPERATIONS 5
#define TEST_TIMEOUT_SECONDS 10

// Test thread data structure
typedef struct {
    int id;
    int priority;
    int operations;
    int errors;
    struct timespec start_time;
    struct timespec end_time;
} test_thread_data_t;

// Test monitor structure
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int current_priority;
    test_thread_data_t *current_thread;
    test_thread_data_t threads[NUM_TEST_THREADS];
    int num_threads;
    int total_errors;
    int total_operations;
} test_monitor_t;

// Global test monitor instance
static test_monitor_t test_monitor;
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Initialize test monitor
static int test_monitor_init(void) {
    if (pthread_mutex_init(&test_monitor.mutex, NULL) != 0) {
        perror("pthread_mutex_init failed");
        return -1;
    }
    
    if (pthread_cond_init(&test_monitor.cond, NULL) != 0) {
        perror("pthread_cond_init failed");
        pthread_mutex_destroy(&test_monitor.mutex);
        return -1;
    }
    
    test_monitor.current_priority = 0;
    test_monitor.current_thread = NULL;
    test_monitor.num_threads = 0;
    test_monitor.total_errors = 0;
    test_monitor.total_operations = 0;
    
    return 0;
}

// Clean up test monitor resources
static void test_monitor_cleanup(void) {
    pthread_cond_destroy(&test_monitor.cond);
    pthread_mutex_destroy(&test_monitor.mutex);
}

// Enter test monitor
static void test_monitor_enter(test_thread_data_t *thread) {
    pthread_mutex_lock(&test_monitor.mutex);
    
    // Priority inheritance
    if (test_monitor.current_thread != NULL && 
        thread->priority > test_monitor.current_priority) {
        test_monitor.current_priority = thread->priority;
    }
    
    // Wait if higher priority thread is running
    while (test_monitor.current_thread != NULL && 
           thread->priority <= test_monitor.current_priority) {
        pthread_cond_wait(&test_monitor.cond, &test_monitor.mutex);
    }
    
    // Enter monitor
    test_monitor.current_thread = thread;
    test_monitor.current_priority = thread->priority;
    
    pthread_mutex_unlock(&test_monitor.mutex);
}

// Exit test monitor
static void test_monitor_exit(test_thread_data_t *thread) {
    pthread_mutex_lock(&test_monitor.mutex);
    
    // Exit monitor
    test_monitor.current_thread = NULL;
    test_monitor.current_priority = 0;
    
    // Signal waiting threads
    pthread_cond_broadcast(&test_monitor.cond);
    
    pthread_mutex_unlock(&test_monitor.mutex);
}

// Test thread function
static void *test_thread_function(void *arg) {
    test_thread_data_t *thread = (test_thread_data_t *)arg;
    
    // Set thread priority
    struct sched_param param;
    param.sched_priority = thread->priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam failed");
        thread->errors++;
        return NULL;
    }
    
    // Record start time
    clock_gettime(CLOCK_MONOTONIC, &thread->start_time);
    
    while (running && thread->operations < TEST_MAX_OPERATIONS) {
        // Enter monitor
        test_monitor_enter(thread);
        if (!running) break;
        
        // Perform test operation
        printf("Test thread %d (priority %d) performing operation %d/%d\n",
               thread->id, thread->priority,
               thread->operations + 1, TEST_MAX_OPERATIONS);
        
        // Verify priority inheritance
        if (test_monitor.current_priority != thread->priority) {
            printf("ERROR: Priority inheritance failed for thread %d\n", thread->id);
            thread->errors++;
        }
        
        sleep(TEST_OPERATION_TIME);
        
        // Exit monitor
        test_monitor_exit(thread);
        thread->operations++;
        
        // Sleep between operations
        sleep(1);
    }
    
    // Record end time
    clock_gettime(CLOCK_MONOTONIC, &thread->end_time);
    
    printf("Test thread %d completed all operations with %d errors\n",
           thread->id, thread->errors);
    return NULL;
}

// Print test results
static void print_test_results(void) {
    printf("\nTest Results:\n");
    printf("Total threads: %d\n", test_monitor.num_threads);
    printf("Total operations: %d\n", test_monitor.total_operations);
    printf("Total errors: %d\n", test_monitor.total_errors);
    
    for (int i = 0; i < test_monitor.num_threads; i++) {
        test_thread_data_t *thread = &test_monitor.threads[i];
        double runtime = (thread->end_time.tv_sec - thread->start_time.tv_sec) +
                        (thread->end_time.tv_nsec - thread->start_time.tv_nsec) / 1e9;
        
        printf("\nThread %d (priority %d):\n", thread->id, thread->priority);
        printf("  Operations: %d\n", thread->operations);
        printf("  Errors: %d\n", thread->errors);
        printf("  Runtime: %.6f seconds\n", runtime);
    }
}

int main(void) {
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize test monitor
    if (test_monitor_init() != 0) {
        return EXIT_FAILURE;
    }
    
    // Create test threads
    pthread_t threads[NUM_TEST_THREADS];
    for (int i = 0; i < NUM_TEST_THREADS; i++) {
        test_thread_data_t *thread = &test_monitor.threads[i];
        thread->id = i;
        thread->priority = (i + 1) * 10;  // Different priorities
        thread->operations = 0;
        thread->errors = 0;
        
        if (pthread_create(&threads[i], NULL, test_thread_function, thread) != 0) {
            perror("pthread_create failed");
            test_monitor_cleanup();
            return EXIT_FAILURE;
        }
        
        test_monitor.num_threads++;
    }
    
    // Wait for all threads to complete or timeout
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    for (int i = 0; i < test_monitor.num_threads; i++) {
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        
        if ((current_time.tv_sec - start_time.tv_sec) > TEST_TIMEOUT_SECONDS) {
            printf("Test timeout after %d seconds\n", TEST_TIMEOUT_SECONDS);
            running = 0;
            break;
        }
        
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join failed");
        }
    }
    
    // Calculate total operations and errors
    for (int i = 0; i < test_monitor.num_threads; i++) {
        test_monitor.total_operations += test_monitor.threads[i].operations;
        test_monitor.total_errors += test_monitor.threads[i].errors;
    }
    
    // Print test results
    print_test_results();
    
    // Clean up
    test_monitor_cleanup();
    
    return (test_monitor.total_errors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
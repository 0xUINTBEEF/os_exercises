/**
 * Thread Priority Management using POSIX Threads
 * 
 * This program demonstrates thread priority management and scheduling using POSIX threads.
 * Features include:
 * - Thread priority setting and management
 * - Priority-based scheduling
 * - Thread state tracking
 * - Performance monitoring
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Constants
#define MAX_THREADS 5
#define MAX_ITERATIONS 1000000
#define PRIORITY_LEVELS 5

// Structure to hold thread data
typedef struct {
    int id;
    int priority;
    int iterations;
    struct timespec start_time;
    struct timespec end_time;
} thread_data_t;

// Global variables
static pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Thread function that performs work based on priority
 * @param arg Thread data containing priority and iterations
 * @return NULL
 */
static void *thread_function(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    struct sched_param param;
    int result;
    int i;
    double dummy = 0.0;

    // Set thread priority
    param.sched_priority = data->priority;
    result = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    if (result != 0) {
        pthread_mutex_lock(&output_mutex);
        fprintf(stderr, "Error setting priority for thread %d: %s\n", 
                data->id, strerror(result));
        pthread_mutex_unlock(&output_mutex);
        return NULL;
    }

    // Record start time
    clock_gettime(CLOCK_MONOTONIC, &data->start_time);

    // Perform work
    for (i = 0; i < data->iterations; i++) {
        dummy += (double)i / (i + 1);
    }

    // Record end time
    clock_gettime(CLOCK_MONOTONIC, &data->end_time);

    // Calculate and display execution time
    double execution_time = (data->end_time.tv_sec - data->start_time.tv_sec) +
                          (data->end_time.tv_nsec - data->start_time.tv_nsec) / 1e9;

    pthread_mutex_lock(&output_mutex);
    printf("Thread %d (Priority %d): Completed %d iterations in %.6f seconds\n",
           data->id, data->priority, data->iterations, execution_time);
    pthread_mutex_unlock(&output_mutex);

    return NULL;
}

/**
 * Main function demonstrating thread priority management
 * @return 0 on success, 1 on error
 */
int main(void) {
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    int result;
    int i;

    // Check if we have sufficient privileges for real-time scheduling
    if (geteuid() != 0) {
        fprintf(stderr, "Warning: Program should be run with root privileges for real-time scheduling\n");
    }

    // Initialize thread data
    for (i = 0; i < MAX_THREADS; i++) {
        thread_data[i].id = i;
        thread_data[i].priority = (i % PRIORITY_LEVELS) + 1;
        thread_data[i].iterations = MAX_ITERATIONS / (i + 1);
    }

    // Create threads
    for (i = 0; i < MAX_THREADS; i++) {
        result = pthread_create(&threads[i], NULL, thread_function, &thread_data[i]);
        if (result != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(result));
            // Cleanup created threads
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            return 1;
        }
    }

    // Wait for all threads to complete
    for (i = 0; i < MAX_THREADS; i++) {
        result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(result));
        }
    }

    return 0;
} 
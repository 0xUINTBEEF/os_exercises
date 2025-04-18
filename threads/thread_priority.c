/**
 * @file thread_priority.c
 * @brief Demonstration of thread priority management
 * 
 * This program demonstrates how to set and manage thread priorities
 * using POSIX threads. It shows how priority affects thread scheduling
 * and execution order.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define NUM_THREADS 3
#define ITERATIONS 5

// Thread data structure
typedef struct {
    int id;
    int priority;
    int iterations;
} thread_data_t;

// Global mutex for synchronized output
pthread_mutex_t output_mutex;

/**
 * @brief Thread function that demonstrates priority scheduling
 * 
 * @param arg Thread data structure
 * @return void* Always returns NULL
 */
void* thread_function(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    
    // Set thread priority
    struct sched_param param;
    param.sched_priority = data->priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam");
    }
    
    // Get actual priority
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &param);
    
    pthread_mutex_lock(&output_mutex);
    printf("Thread %d started with priority %d\n", data->id, param.sched_priority);
    pthread_mutex_unlock(&output_mutex);
    
    // Perform work
    for (int i = 0; i < data->iterations; i++) {
        pthread_mutex_lock(&output_mutex);
        printf("Thread %d (priority %d) iteration %d\n", 
               data->id, param.sched_priority, i + 1);
        pthread_mutex_unlock(&output_mutex);
        
        // Simulate work
        for (int j = 0; j < 1000000; j++) {
            // Busy wait
        }
    }
    
    pthread_mutex_lock(&output_mutex);
    printf("Thread %d completed\n", data->id);
    pthread_mutex_unlock(&output_mutex);
    
    return NULL;
}

/**
 * @brief Main function
 */
int main(void) {
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];
    
    // Initialize mutex
    pthread_mutex_init(&output_mutex, NULL);
    
    // Create threads with different priorities
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].id = i;
        thread_data[i].priority = (i + 1) * 10;  // Different priorities
        thread_data[i].iterations = ITERATIONS;
        
        if (pthread_create(&threads[i], NULL, thread_function, &thread_data[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }
    
    // Wait for threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Clean up
    pthread_mutex_destroy(&output_mutex);
    
    return 0;
} 
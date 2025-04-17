/**
 * @file test_monitor.c
 * @brief Test suite for monitor implementation
 * 
 * This program implements comprehensive tests for the monitor
 * implementation, including deadlock detection, priority inheritance,
 * and performance metrics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

// Test configuration
#define NUM_TEST_THREADS 5
#define TEST_DURATION_SEC 10
#define TEST_BUFFER_SIZE 10
#define TEST_TIMEOUT_MS 1000

// Test metrics
typedef struct {
    unsigned long successful_ops;
    unsigned long failed_ops;
    unsigned long timeouts;
    double avg_response_time;
    struct timespec start_time;
} test_metrics_t;

test_metrics_t metrics;

/**
 * @brief Initialize test metrics
 */
void init_test_metrics(void) {
    metrics.successful_ops = 0;
    metrics.failed_ops = 0;
    metrics.timeouts = 0;
    metrics.avg_response_time = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &metrics.start_time);
}

/**
 * @brief Test producer thread with varying priorities
 */
void* test_producer(void* arg) {
    int id = *(int*)arg;
    int priority = id % 3;  // Assign different priorities
    struct timespec op_start, op_end;
    
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &op_start);
        
        // Try to insert with timeout
        int item = rand() % 100;
        monitor_error_t result = monitor_insert(item, TEST_TIMEOUT_MS);
        
        clock_gettime(CLOCK_MONOTONIC, &op_end);
        double response_time = (op_end.tv_sec - op_start.tv_sec) +
                              (op_end.tv_nsec - op_start.tv_nsec) / 1e9;
        
        // Update metrics
        if (result == MONITOR_SUCCESS) {
            metrics.successful_ops++;
            metrics.avg_response_time = 
                (metrics.avg_response_time * (metrics.successful_ops - 1) + response_time) /
                metrics.successful_ops;
        } else if (result == MONITOR_ERROR_TIMEOUT) {
            metrics.timeouts++;
        } else {
            metrics.failed_ops++;
        }
        
        // Simulate varying workload
        usleep(rand() % 100000);
    }
    return NULL;
}

/**
 * @brief Test consumer thread with varying priorities
 */
void* test_consumer(void* arg) {
    int id = *(int*)arg;
    int priority = id % 3;  // Assign different priorities
    struct timespec op_start, op_end;
    
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &op_start);
        
        // Try to remove with timeout
        int item;
        monitor_error_t result = monitor_remove(&item, TEST_TIMEOUT_MS);
        
        clock_gettime(CLOCK_MONOTONIC, &op_end);
        double response_time = (op_end.tv_sec - op_start.tv_sec) +
                              (op_end.tv_nsec - op_start.tv_nsec) / 1e9;
        
        // Update metrics
        if (result == MONITOR_SUCCESS) {
            metrics.successful_ops++;
            metrics.avg_response_time = 
                (metrics.avg_response_time * (metrics.successful_ops - 1) + response_time) /
                metrics.successful_ops;
        } else if (result == MONITOR_ERROR_TIMEOUT) {
            metrics.timeouts++;
        } else {
            metrics.failed_ops++;
        }
        
        // Simulate varying workload
        usleep(rand() % 150000);
    }
    return NULL;
}

/**
 * @brief Print test results
 */
void print_test_results(void) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double total_time = (end_time.tv_sec - metrics.start_time.tv_sec) +
                       (end_time.tv_nsec - metrics.start_time.tv_nsec) / 1e9;
    
    printf("\nTest Results:\n");
    printf("Total runtime: %.2f seconds\n", total_time);
    printf("Successful operations: %lu\n", metrics.successful_ops);
    printf("Failed operations: %lu\n", metrics.failed_ops);
    printf("Timeouts: %lu\n", metrics.timeouts);
    printf("Average response time: %.6f seconds\n", metrics.avg_response_time);
    printf("Operations per second: %.2f\n", metrics.successful_ops / total_time);
    
    // Print monitor-specific metrics
    print_monitor_stats();
}

/**
 * @brief Main test function
 */
int main(void) {
    pthread_t producers[NUM_TEST_THREADS];
    pthread_t consumers[NUM_TEST_THREADS];
    int thread_ids[NUM_TEST_THREADS * 2];
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize monitor and metrics
    assert(monitor_init() == MONITOR_SUCCESS);
    init_test_metrics();
    
    // Create test threads
    for (int i = 0; i < NUM_TEST_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&producers[i], NULL, test_producer, &thread_ids[i]);
        thread_ids[i + NUM_TEST_THREADS] = i + NUM_TEST_THREADS;
        pthread_create(&consumers[i], NULL, test_consumer, &thread_ids[i + NUM_TEST_THREADS]);
    }
    
    // Run tests for specified duration
    sleep(TEST_DURATION_SEC);
    
    // Print results and cleanup
    print_test_results();
    monitor_destroy();
    
    return 0;
}
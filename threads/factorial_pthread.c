/**
 * Factorial Calculation using POSIX Threads
 * 
 * This program demonstrates parallel computation of factorial using multiple threads.
 * Features include:
 * - Parallel computation of large factorials
 * - Thread-safe operations
 * - Dynamic thread management
 * - Error handling and resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

// Constants
#define MAX_THREADS 4
#define CHUNK_SIZE 1000

// Structure to hold thread data
typedef struct {
    int start;
    int end;
    unsigned long long result;
} thread_data_t;

// Global variables
static pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned long long final_result = 1;

/**
 * Calculate factorial for a range of numbers
 * @param arg Thread data containing range and result
 * @return NULL
 */
static void *factorial_range(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    unsigned long long local_result = 1;

    // Calculate factorial for the assigned range
    for (int i = data->start; i <= data->end; i++) {
        local_result *= i;
    }

    // Update global result safely
    pthread_mutex_lock(&result_mutex);
    final_result *= local_result;
    pthread_mutex_unlock(&result_mutex);

    return NULL;
}

/**
 * Calculate factorial of a number using multiple threads
 * @param n Number to calculate factorial of
 * @return Factorial result, 0 on error
 */
unsigned long long calculate_factorial(int n) {
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    int num_threads;
    int chunk_size;
    int result;
    int i;

    // Input validation
    if (n < 0) {
        fprintf(stderr, "Error: Factorial is not defined for negative numbers\n");
        return 0;
    }

    if (n == 0 || n == 1) {
        return 1;
    }

    // Determine number of threads and chunk size
    num_threads = (n < MAX_THREADS) ? n : MAX_THREADS;
    chunk_size = n / num_threads;

    // Initialize thread data
    for (i = 0; i < num_threads; i++) {
        thread_data[i].start = (i * chunk_size) + 1;
        thread_data[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
        thread_data[i].result = 1;
    }

    // Create threads
    for (i = 0; i < num_threads; i++) {
        result = pthread_create(&threads[i], NULL, factorial_range, &thread_data[i]);
        if (result != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(result));
            // Cleanup created threads
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            return 0;
        }
    }

    // Wait for all threads to complete
    for (i = 0; i < num_threads; i++) {
        result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(result));
        }
    }

    return final_result;
}

/**
 * Main function demonstrating factorial calculation
 * @return 0 on success, 1 on error
 */
int main(void) {
    int n;
    unsigned long long result;

    printf("Enter a number to calculate factorial: ");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Error: Invalid input\n");
        return 1;
    }

    result = calculate_factorial(n);
    if (result == 0) {
        return 1;
    }

    printf("Factorial of %d is %llu\n", n, result);
    return 0;
}
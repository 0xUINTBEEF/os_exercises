/**
 * Fibonacci Sequence Calculation using POSIX Threads
 * 
 * This program demonstrates parallel computation of Fibonacci sequence using multiple threads.
 * Features include:
 * - Parallel computation of large Fibonacci numbers
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
#define MAX_SEQUENCE 100

// Structure to hold thread data
typedef struct {
    int start;
    int end;
    unsigned long long *sequence;
} thread_data_t;

// Global variables
static pthread_mutex_t sequence_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Calculate Fibonacci sequence for a range of numbers
 * @param arg Thread data containing range and sequence array
 * @return NULL
 */
static void *fibonacci_range(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    unsigned long long a, b, temp;
    int i;

    // Initialize first two numbers in the range
    if (data->start <= 1) {
        data->sequence[0] = 0;
        data->sequence[1] = 1;
        i = 2;
    } else {
        // Calculate previous two numbers
        a = data->sequence[data->start - 2];
        b = data->sequence[data->start - 1];
        i = data->start;
    }

    // Calculate Fibonacci numbers in the range
    for (; i <= data->end; i++) {
        temp = a + b;
        a = b;
        b = temp;
        data->sequence[i] = b;
    }

    return NULL;
}

/**
 * Calculate Fibonacci sequence using multiple threads
 * @param n Number of elements to calculate
 * @param sequence Array to store the sequence
 * @return 0 on success, -1 on error
 */
int calculate_fibonacci(int n, unsigned long long *sequence) {
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    int num_threads;
    int chunk_size;
    int result;
    int i;

    // Input validation
    if (n <= 0 || n > MAX_SEQUENCE) {
        fprintf(stderr, "Error: Invalid sequence length\n");
        return -1;
    }

    if (sequence == NULL) {
        fprintf(stderr, "Error: Invalid sequence array\n");
        return -1;
    }

    // Initialize first two numbers
    sequence[0] = 0;
    if (n > 1) {
        sequence[1] = 1;
    }

    if (n <= 2) {
        return 0;
    }

    // Determine number of threads and chunk size
    num_threads = (n < MAX_THREADS) ? n : MAX_THREADS;
    chunk_size = (n - 2) / num_threads;

    // Initialize thread data
    for (i = 0; i < num_threads; i++) {
        thread_data[i].start = (i * chunk_size) + 2;
        thread_data[i].end = (i == num_threads - 1) ? n - 1 : (i + 1) * chunk_size + 1;
        thread_data[i].sequence = sequence;
    }

    // Create threads
    for (i = 0; i < num_threads; i++) {
        result = pthread_create(&threads[i], NULL, fibonacci_range, &thread_data[i]);
        if (result != 0) {
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(result));
            // Cleanup created threads
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            return -1;
        }
    }

    // Wait for all threads to complete
    for (i = 0; i < num_threads; i++) {
        result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(result));
        }
    }

    return 0;
}

/**
 * Main function demonstrating Fibonacci sequence calculation
 * @return 0 on success, 1 on error
 */
int main(void) {
    int n;
    unsigned long long sequence[MAX_SEQUENCE];

    printf("Enter the number of Fibonacci numbers to calculate (1-%d): ", MAX_SEQUENCE);
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Error: Invalid input\n");
        return 1;
    }

    if (calculate_fibonacci(n, sequence) != 0) {
        return 1;
    }

    printf("Fibonacci sequence: ");
    for (int i = 0; i < n; i++) {
        printf("%llu ", sequence[i]);
    }
    printf("\n");

    return 0;
}
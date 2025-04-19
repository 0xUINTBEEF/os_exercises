/**
 * Array Statistics with Threads
 * 
 * This program calculates the average, minimum, and maximum values
 * of an array using separate threads for each calculation.
 * 
 * Features:
 * - Parallel computation of statistics
 * - Thread-safe memory management
 * - Error handling
 * - Input validation
 * - Performance monitoring
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <time.h>

// Constants
#define MAX_ELEMENTS 1000000
#define MAX_THREADS 3

// Thread arguments structure
typedef struct {
    int *array;
    int n;
    double result;
    struct timespec start_time;
    struct timespec end_time;
} thread_args_t;

// Thread function prototypes
static void *average_thread(void *args);
static void *min_val_thread(void *args);
static void *max_val_thread(void *args);

// Calculate average in a separate thread
static void *average_thread(void *args) {
    thread_args_t *t_args = (thread_args_t *)args;
    clock_gettime(CLOCK_MONOTONIC, &t_args->start_time);
    
    double sum = 0.0;
    for (int i = 0; i < t_args->n; i++) {
        sum += t_args->array[i];
    }
    t_args->result = sum / t_args->n;
    
    clock_gettime(CLOCK_MONOTONIC, &t_args->end_time);
    return NULL;
}

// Calculate minimum value in a separate thread
static void *min_val_thread(void *args) {
    thread_args_t *t_args = (thread_args_t *)args;
    clock_gettime(CLOCK_MONOTONIC, &t_args->start_time);
    
    t_args->result = t_args->array[0];
    for (int i = 1; i < t_args->n; i++) {
        if (t_args->array[i] < t_args->result) {
            t_args->result = t_args->array[i];
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &t_args->end_time);
    return NULL;
}

// Calculate maximum value in a separate thread
static void *max_val_thread(void *args) {
    thread_args_t *t_args = (thread_args_t *)args;
    clock_gettime(CLOCK_MONOTONIC, &t_args->start_time);
    
    t_args->result = t_args->array[0];
    for (int i = 1; i < t_args->n; i++) {
        if (t_args->array[i] > t_args->result) {
            t_args->result = t_args->array[i];
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &t_args->end_time);
    return NULL;
}

// Print thread statistics
static void print_thread_stats(thread_args_t *args, const char *operation) {
    double runtime = (args->end_time.tv_sec - args->start_time.tv_sec) +
                    (args->end_time.tv_nsec - args->start_time.tv_nsec) / 1e9;
    printf("%s: %.2f (calculated in %.6f seconds)\n",
           operation, args->result, runtime);
}

int main(void) {
    int num_elements;
    int *array = NULL;
    pthread_t threads[MAX_THREADS];
    thread_args_t thread_args[MAX_THREADS];
    int ret;

    // Get number of elements
    printf("Enter number of elements (max %d): ", MAX_ELEMENTS);
    if (scanf("%d", &num_elements) != 1 || num_elements <= 0 || num_elements > MAX_ELEMENTS) {
        fprintf(stderr, "Invalid number of elements\n");
        return EXIT_FAILURE;
    }

    // Allocate array
    array = malloc(num_elements * sizeof(int));
    if (!array) {
        perror("Failed to allocate array");
        return EXIT_FAILURE;
    }

    // Get array elements
    printf("Enter %d elements:\n", num_elements);
    for (int i = 0; i < num_elements; i++) {
        if (scanf("%d", &array[i]) != 1) {
            fprintf(stderr, "Invalid input\n");
            free(array);
            return EXIT_FAILURE;
        }
    }

    // Initialize thread arguments
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_args[i].array = array;
        thread_args[i].n = num_elements;
    }

    // Create threads
    if ((ret = pthread_create(&threads[0], NULL, average_thread, &thread_args[0])) != 0) {
        fprintf(stderr, "Failed to create average thread: %s\n", strerror(ret));
        free(array);
        return EXIT_FAILURE;
    }

    if ((ret = pthread_create(&threads[1], NULL, min_val_thread, &thread_args[1])) != 0) {
        fprintf(stderr, "Failed to create min thread: %s\n", strerror(ret));
        pthread_join(threads[0], NULL);
        free(array);
        return EXIT_FAILURE;
    }

    if ((ret = pthread_create(&threads[2], NULL, max_val_thread, &thread_args[2])) != 0) {
        fprintf(stderr, "Failed to create max thread: %s\n", strerror(ret));
        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);
        free(array);
        return EXIT_FAILURE;
    }

    // Wait for threads to complete
    for (int i = 0; i < MAX_THREADS; i++) {
        if ((ret = pthread_join(threads[i], NULL)) != 0) {
            fprintf(stderr, "Failed to join thread %d: %s\n", i, strerror(ret));
        }
    }

    // Print results
    printf("\nArray Statistics:\n");
    print_thread_stats(&thread_args[0], "Average");
    print_thread_stats(&thread_args[1], "Minimum");
    print_thread_stats(&thread_args[2], "Maximum");

    // Clean up
    free(array);
    return EXIT_SUCCESS;
}


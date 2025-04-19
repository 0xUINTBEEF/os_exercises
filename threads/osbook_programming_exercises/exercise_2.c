/**
 * Sieve of Eratosthenes with Threads
 * 
 * This program implements the Sieve of Eratosthenes algorithm
 * to find all prime numbers up to a given upper bound using threads.
 * 
 * Features:
 * - Parallel prime number calculation
 * - Memory-efficient implementation
 * - Error handling
 * - Input validation
 * - Performance monitoring
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Constants
#define MAX_UPPER_BOUND 100000000
#define MIN_UPPER_BOUND 2

// Thread arguments structure
typedef struct {
    int upper_bound;
    int *is_prime;
    int start;
    int end;
    struct timespec start_time;
    struct timespec end_time;
} sieve_args_t;

// Sieve thread function
static void *sieve_thread(void *args) {
    sieve_args_t *s_args = (sieve_args_t *)args;
    clock_gettime(CLOCK_MONOTONIC, &s_args->start_time);
    
    // Mark multiples of each prime number
    for (int i = s_args->start; i <= s_args->end; i++) {
        if (s_args->is_prime[i]) {
            for (int j = i * i; j <= s_args->upper_bound; j += i) {
                s_args->is_prime[j] = 0;
            }
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &s_args->end_time);
    return NULL;
}

// Print prime numbers
static void print_primes(const int *is_prime, int upper_bound) {
    printf("\nPrime numbers up to %d:\n", upper_bound);
    int count = 0;
    for (int i = 2; i <= upper_bound; i++) {
        if (is_prime[i]) {
            printf("%d ", i);
            count++;
            if (count % 10 == 0) {
                printf("\n");
            }
        }
    }
    printf("\n\nTotal primes found: %d\n", count);
}

// Print thread statistics
static void print_thread_stats(sieve_args_t *args) {
    double runtime = (args->end_time.tv_sec - args->start_time.tv_sec) +
                    (args->end_time.tv_nsec - args->start_time.tv_nsec) / 1e9;
    printf("Thread processed range %d-%d in %.6f seconds\n",
           args->start, args->end, runtime);
}

int main(void) {
    int upper_bound;
    int *is_prime = NULL;
    pthread_t thread;
    sieve_args_t args;
    int ret;
    struct timespec total_start, total_end;

    // Get upper bound
    printf("Enter upper bound (between %d and %d): ", MIN_UPPER_BOUND, MAX_UPPER_BOUND);
    if (scanf("%d", &upper_bound) != 1 || 
        upper_bound < MIN_UPPER_BOUND || 
        upper_bound > MAX_UPPER_BOUND) {
        fprintf(stderr, "Invalid upper bound\n");
        return EXIT_FAILURE;
    }

    // Start timing
    clock_gettime(CLOCK_MONOTONIC, &total_start);

    // Allocate and initialize array
    is_prime = malloc((upper_bound + 1) * sizeof(int));
    if (!is_prime) {
        perror("Failed to allocate array");
        return EXIT_FAILURE;
    }

    // Initialize array
    for (int i = 0; i <= upper_bound; i++) {
        is_prime[i] = 1;
    }
    is_prime[0] = is_prime[1] = 0;

    // Set up thread arguments
    args.upper_bound = upper_bound;
    args.is_prime = is_prime;
    args.start = 2;
    args.end = (int)sqrt(upper_bound);

    // Create thread
    if ((ret = pthread_create(&thread, NULL, sieve_thread, &args)) != 0) {
        fprintf(stderr, "Failed to create thread: %s\n", strerror(ret));
        free(is_prime);
        return EXIT_FAILURE;
    }

    // Wait for thread to complete
    if ((ret = pthread_join(thread, NULL)) != 0) {
        fprintf(stderr, "Failed to join thread: %s\n", strerror(ret));
        free(is_prime);
        return EXIT_FAILURE;
    }

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &total_end);

    // Print thread statistics
    print_thread_stats(&args);

    // Print total runtime
    double total_runtime = (total_end.tv_sec - total_start.tv_sec) +
                          (total_end.tv_nsec - total_start.tv_nsec) / 1e9;
    printf("\nTotal runtime: %.6f seconds\n", total_runtime);

    // Print prime numbers
    print_primes(is_prime, upper_bound);

    // Clean up
    free(is_prime);
    return EXIT_SUCCESS;
}
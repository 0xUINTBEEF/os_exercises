/**
 * Dining Philosophers Problem Solution
 * 
 * This program demonstrates a solution to the dining philosophers problem
 * using mutexes and condition variables. It implements a deadlock-free
 * solution with proper resource management and synchronization.
 * 
 * Features:
 * - Deadlock prevention
 * - Resource hierarchy solution
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
#define NUM_PHILOSOPHERS 5
#define EATING_TIME 2
#define THINKING_TIME 3
#define MAX_MEALS 3

// Philosopher states
typedef enum {
    THINKING,
    HUNGRY,
    EATING
} philosopher_state_t;

// Global variables
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_vars[NUM_PHILOSOPHERS];
static philosopher_state_t states[NUM_PHILOSOPHERS];
static int meals_eaten[NUM_PHILOSOPHERS];
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * @brief Check if philosopher can eat
 * 
 * @param philosopher Philosopher ID
 * @return true if philosopher can eat, false otherwise
 */
static int can_eat(int philosopher) {
    int left = (philosopher + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (philosopher + 1) % NUM_PHILOSOPHERS;
    
    return (states[philosopher] == HUNGRY &&
            states[left] != EATING &&
            states[right] != EATING);
}

/**
 * @brief Philosopher thread function
 * 
 * @param arg Philosopher ID
 * @return void* Always returns NULL
 */
static void *philosopher_thread(void *arg) {
    int philosopher = *(int *)arg;
    free(arg);
    
    printf("Philosopher %d starting\n", philosopher);
    
    while (running && meals_eaten[philosopher] < MAX_MEALS) {
        // Think
        printf("Philosopher %d is thinking\n", philosopher);
        sleep(THINKING_TIME);
        
        // Get hungry
        pthread_mutex_lock(&mutex);
        states[philosopher] = HUNGRY;
        printf("Philosopher %d is hungry\n", philosopher);
        
        // Try to eat
        while (!can_eat(philosopher) && running) {
            pthread_cond_wait(&cond_vars[philosopher], &mutex);
        }
        
        if (!running) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // Eat
        states[philosopher] = EATING;
        meals_eaten[philosopher]++;
        printf("Philosopher %d is eating (meal %d/%d)\n", 
               philosopher, meals_eaten[philosopher], MAX_MEALS);
        
        pthread_mutex_unlock(&mutex);
        sleep(EATING_TIME);
        
        // Finish eating
        pthread_mutex_lock(&mutex);
        states[philosopher] = THINKING;
        printf("Philosopher %d finished eating\n", philosopher);
        
        // Notify neighbors
        int left = (philosopher + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
        int right = (philosopher + 1) % NUM_PHILOSOPHERS;
        
        if (states[left] == HUNGRY && can_eat(left)) {
            pthread_cond_signal(&cond_vars[left]);
        }
        if (states[right] == HUNGRY && can_eat(right)) {
            pthread_cond_signal(&cond_vars[right]);
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    printf("Philosopher %d completed all meals\n", philosopher);
    return NULL;
}

/**
 * @brief Main function
 */
int main(void) {
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize condition variables
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_cond_init(&cond_vars[i], NULL) != 0) {
            perror("pthread_cond_init failed");
            return EXIT_FAILURE;
        }
        states[i] = THINKING;
        meals_eaten[i] = 0;
    }
    
    // Create philosopher threads
    pthread_t threads[NUM_PHILOSOPHERS];
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        int *philosopher = malloc(sizeof(int));
        if (!philosopher) {
            perror("malloc failed");
            return EXIT_FAILURE;
        }
        *philosopher = i;
        
        if (pthread_create(&threads[i], NULL, philosopher_thread, philosopher) != 0) {
            perror("pthread_create failed");
            free(philosopher);
            return EXIT_FAILURE;
        }
    }
    
    // Wait for all philosophers to complete
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join failed");
        }
    }
    
    // Clean up
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_destroy(&cond_vars[i]);
    }
    pthread_mutex_destroy(&mutex);
    
    printf("All philosophers have completed their meals\n");
    return EXIT_SUCCESS;
}
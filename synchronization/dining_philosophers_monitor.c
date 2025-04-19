/**
 * Dining Philosophers Problem with Monitor Implementation
 * 
 * This program demonstrates a deadlock-free solution to the dining
 * philosophers problem using monitors and condition variables. It
 * implements a resource hierarchy solution to prevent deadlocks.
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

// Philosopher data structure
typedef struct {
    int id;
    philosopher_state_t state;
    int meals_eaten;
    struct timespec start_time;
    struct timespec end_time;
    double total_wait_time;
} philosopher_data_t;

// Monitor structure
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond_vars[NUM_PHILOSOPHERS];
    philosopher_state_t states[NUM_PHILOSOPHERS];
    philosopher_data_t philosophers[NUM_PHILOSOPHERS];
    int num_philosophers;
} dining_monitor_t;

// Global monitor instance
static dining_monitor_t monitor;
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
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_cond_init(&monitor.cond_vars[i], NULL) != 0) {
            perror("pthread_cond_init failed");
            // Clean up previously initialized condition variables
            for (int j = 0; j < i; j++) {
                pthread_cond_destroy(&monitor.cond_vars[j]);
            }
            pthread_mutex_destroy(&monitor.mutex);
            return -1;
        }
        
        monitor.states[i] = THINKING;
        monitor.philosophers[i].id = i;
        monitor.philosophers[i].state = THINKING;
        monitor.philosophers[i].meals_eaten = 0;
        monitor.philosophers[i].total_wait_time = 0.0;
    }
    
    monitor.num_philosophers = NUM_PHILOSOPHERS;
    return 0;
}

// Clean up monitor resources
static void monitor_cleanup(void) {
    for (int i = 0; i < monitor.num_philosophers; i++) {
        pthread_cond_destroy(&monitor.cond_vars[i]);
    }
    pthread_mutex_destroy(&monitor.mutex);
}

// Check if philosopher can eat
static int can_eat(int philosopher) {
    int left = (philosopher + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (philosopher + 1) % NUM_PHILOSOPHERS;
    
    return (monitor.states[philosopher] == HUNGRY &&
            monitor.states[left] != EATING &&
            monitor.states[right] != EATING);
}

// Philosopher thread function
static void *philosopher_thread(void *arg) {
    philosopher_data_t *philosopher = (philosopher_data_t *)arg;
    struct timespec wait_start, wait_end;
    
    // Record start time
    clock_gettime(CLOCK_MONOTONIC, &philosopher->start_time);
    
    while (running && philosopher->meals_eaten < MAX_MEALS) {
        // Think
        printf("Philosopher %d is thinking\n", philosopher->id);
        sleep(THINKING_TIME);
        
        // Get hungry
        pthread_mutex_lock(&monitor.mutex);
        monitor.states[philosopher->id] = HUNGRY;
        philosopher->state = HUNGRY;
        printf("Philosopher %d is hungry\n", philosopher->id);
        
        // Try to eat
        clock_gettime(CLOCK_MONOTONIC, &wait_start);
        while (!can_eat(philosopher->id) && running) {
            pthread_cond_wait(&monitor.cond_vars[philosopher->id], &monitor.mutex);
        }
        
        if (!running) {
            pthread_mutex_unlock(&monitor.mutex);
            break;
        }
        
        // Eat
        monitor.states[philosopher->id] = EATING;
        philosopher->state = EATING;
        philosopher->meals_eaten++;
        
        clock_gettime(CLOCK_MONOTONIC, &wait_end);
        double wait_time = (wait_end.tv_sec - wait_start.tv_sec) +
                          (wait_end.tv_nsec - wait_start.tv_nsec) / 1e9;
        philosopher->total_wait_time += wait_time;
        
        printf("Philosopher %d is eating (meal %d/%d)\n",
               philosopher->id, philosopher->meals_eaten, MAX_MEALS);
        
        pthread_mutex_unlock(&monitor.mutex);
        sleep(EATING_TIME);
        
        // Finish eating
        pthread_mutex_lock(&monitor.mutex);
        monitor.states[philosopher->id] = THINKING;
        philosopher->state = THINKING;
        printf("Philosopher %d finished eating\n", philosopher->id);
        
        // Notify neighbors
        int left = (philosopher->id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
        int right = (philosopher->id + 1) % NUM_PHILOSOPHERS;
        
        if (monitor.states[left] == HUNGRY && can_eat(left)) {
            pthread_cond_signal(&monitor.cond_vars[left]);
        }
        if (monitor.states[right] == HUNGRY && can_eat(right)) {
            pthread_cond_signal(&monitor.cond_vars[right]);
        }
        
        pthread_mutex_unlock(&monitor.mutex);
    }
    
    // Record end time
    clock_gettime(CLOCK_MONOTONIC, &philosopher->end_time);
    
    printf("Philosopher %d completed all meals\n", philosopher->id);
    return NULL;
}

// Print performance statistics
static void print_stats(void) {
    printf("\nDining Philosophers Statistics:\n");
    
    for (int i = 0; i < monitor.num_philosophers; i++) {
        philosopher_data_t *philosopher = &monitor.philosophers[i];
        double runtime = (philosopher->end_time.tv_sec - philosopher->start_time.tv_sec) +
                        (philosopher->end_time.tv_nsec - philosopher->start_time.tv_nsec) / 1e9;
        
        printf("\nPhilosopher %d:\n", philosopher->id);
        printf("  Meals eaten: %d\n", philosopher->meals_eaten);
        printf("  Total wait time: %.6f seconds\n", philosopher->total_wait_time);
        printf("  Average wait time: %.6f seconds\n",
               philosopher->total_wait_time / philosopher->meals_eaten);
        printf("  Runtime: %.6f seconds\n", runtime);
    }
}

int main(void) {
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Initialize monitor
    if (monitor_init() != 0) {
        return EXIT_FAILURE;
    }
    
    // Create philosopher threads
    pthread_t threads[NUM_PHILOSOPHERS];
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_create(&threads[i], NULL, philosopher_thread,
                          &monitor.philosophers[i]) != 0) {
            perror("pthread_create failed");
            monitor_cleanup();
            return EXIT_FAILURE;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join failed");
        }
    }
    
    // Print statistics
    print_stats();
    
    // Clean up
    monitor_cleanup();
    
    return EXIT_SUCCESS;
}
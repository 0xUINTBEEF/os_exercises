/**
 * @file dining_philosophers_monitor.c
 * @brief Implementation of dining philosophers problem using monitors
 * 
 * This program demonstrates a deadlock-free solution to the dining
 * philosophers problem using monitors and condition variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2

typedef struct {
    int state[NUM_PHILOSOPHERS];
    pthread_mutex_t mutex;
    pthread_cond_t can_eat[NUM_PHILOSOPHERS];
    
    // Performance metrics
    unsigned long meals_eaten[NUM_PHILOSOPHERS];
    double avg_wait_time[NUM_PHILOSOPHERS];
    struct timespec start_time;
} dining_monitor_t;

dining_monitor_t monitor;

/**
 * @brief Initialize the dining monitor
 */
void monitor_init(void) {
    pthread_mutex_init(&monitor.mutex, NULL);
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        monitor.state[i] = THINKING;
        pthread_cond_init(&monitor.can_eat[i], NULL);
        monitor.meals_eaten[i] = 0;
        monitor.avg_wait_time[i] = 0.0;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &monitor.start_time);
}

/**
 * @brief Check if philosopher can eat
 */
static bool can_eat(int phil_id) {
    int left = (phil_id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (phil_id + 1) % NUM_PHILOSOPHERS;
    return (monitor.state[phil_id] == HUNGRY &&
            monitor.state[left] != EATING &&
            monitor.state[right] != EATING);
}

/**
 * @brief Try to acquire forks and eat
 */
void pickup_forks(int phil_id) {
    struct timespec wait_start, wait_end;
    clock_gettime(CLOCK_MONOTONIC, &wait_start);
    
    pthread_mutex_lock(&monitor.mutex);
    
    monitor.state[phil_id] = HUNGRY;
    printf("Philosopher %d is hungry\n", phil_id);
    
    // Wait until can eat
    while (!can_eat(phil_id)) {
        pthread_cond_wait(&monitor.can_eat[phil_id], &monitor.mutex);
    }
    
    monitor.state[phil_id] = EATING;
    clock_gettime(CLOCK_MONOTONIC, &wait_end);
    
    // Update metrics
    double wait_time = (wait_end.tv_sec - wait_start.tv_sec) +
                      (wait_end.tv_nsec - wait_start.tv_nsec) / 1e9;
    monitor.avg_wait_time[phil_id] = 
        (monitor.avg_wait_time[phil_id] * monitor.meals_eaten[phil_id] + wait_time) /
        (monitor.meals_eaten[phil_id] + 1);
    monitor.meals_eaten[phil_id]++;
    
    printf("Philosopher %d is eating\n", phil_id);
    pthread_mutex_unlock(&monitor.mutex);
}

/**
 * @brief Release forks after eating
 */
void return_forks(int phil_id) {
    pthread_mutex_lock(&monitor.mutex);
    
    monitor.state[phil_id] = THINKING;
    printf("Philosopher %d is thinking\n", phil_id);
    
    // Check if neighbors can eat
    int left = (phil_id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (phil_id + 1) % NUM_PHILOSOPHERS;
    
    if (can_eat(left)) {
        pthread_cond_signal(&monitor.can_eat[left]);
    }
    if (can_eat(right)) {
        pthread_cond_signal(&monitor.can_eat[right]);
    }
    
    pthread_mutex_unlock(&monitor.mutex);
}

/**
 * @brief Print performance statistics
 */
void print_stats(void) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double total_time = (end_time.tv_sec - monitor.start_time.tv_sec) +
                       (end_time.tv_nsec - monitor.start_time.tv_nsec) / 1e9;
    
    printf("\nDining Philosophers Statistics:\n");
    printf("Total runtime: %.2f seconds\n", total_time);
    
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d:\n", i);
        printf("  Meals eaten: %lu\n", monitor.meals_eaten[i]);
        printf("  Average wait time: %.6f seconds\n", monitor.avg_wait_time[i]);
        printf("  Meals per minute: %.2f\n", 
               (monitor.meals_eaten[i] / total_time) * 60);
    }
}

/**
 * @brief Philosopher thread function
 */
void* philosopher(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        // Think for a while
        usleep(rand() % 1000000);
        
        // Get hungry and try to eat
        pickup_forks(id);
        
        // Eat for a while
        usleep(rand() % 500000);
        
        // Finish eating and return forks
        return_forks(id);
    }
    return NULL;
}

/**
 * @brief Main function demonstrating dining philosophers
 */
int main(void) {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize monitor
    monitor_init();
    
    // Create philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
    }
    
    // Let the simulation run for 30 seconds
    sleep(30);
    
    // Print statistics and exit
    print_stats();
    return 0;
}
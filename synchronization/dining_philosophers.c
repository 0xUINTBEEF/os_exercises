/**
 * @file dining_philosophers.c
 * @brief Implementation of the dining philosophers problem using monitors
 * 
 * This program demonstrates a deadlock-free solution to the dining
 * philosophers problem using monitors for synchronization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_PHILOSOPHERS 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2

/**
 * @brief Monitor structure for dining philosophers
 */
typedef struct {
    int state[NUM_PHILOSOPHERS];
    pthread_mutex_t mutex;
    pthread_cond_t can_eat[NUM_PHILOSOPHERS];
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
    }
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
    pthread_mutex_lock(&monitor.mutex);
    
    monitor.state[phil_id] = HUNGRY;
    printf("Philosopher %d is hungry\n", phil_id);
    
    // Try to eat if possible
    while (!can_eat(phil_id)) {
        pthread_cond_wait(&monitor.can_eat[phil_id], &monitor.mutex);
    }
    
    monitor.state[phil_id] = EATING;
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
 * @brief Philosopher thread function
 */
void* philosopher(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        // Think for a while
        printf("Philosopher %d is thinking\n", id);
        usleep(rand() % 1000000);
        
        // Get hungry and try to eat
        pickup_forks(id);
        
        // Eat for a while
        usleep(rand() % 1000000);
        
        // Done eating, put forks back
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
    
    // Wait for threads to complete (they won't in this example)
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    return 0;
}
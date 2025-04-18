/**
 * @file dining_philosophers.c
 * @brief Implementation of the dining philosophers problem
 * 
 * This program demonstrates a solution to the dining philosophers problem
 * using mutexes and condition variables. It includes deadlock prevention
 * and resource hierarchy solution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_PHILOSOPHERS 5
#define EATING_TIME 2
#define THINKING_TIME 3

// Philosopher states
typedef enum {
    THINKING,
    HUNGRY,
    EATING
} philosopher_state_t;

// Global variables
pthread_mutex_t mutex;
pthread_cond_t condition[NUM_PHILOSOPHERS];
philosopher_state_t states[NUM_PHILOSOPHERS];
pthread_t philosophers[NUM_PHILOSOPHERS];
int philosopher_ids[NUM_PHILOSOPHERS];

/**
 * @brief Check if philosopher can eat
 * 
 * @param id Philosopher ID
 * @return true if philosopher can eat, false otherwise
 */
bool can_eat(int id) {
    int left = (id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    return (states[id] == HUNGRY &&
            states[left] != EATING &&
            states[right] != EATING);
}

/**
 * @brief Try to pick up chopsticks
 * 
 * @param id Philosopher ID
 */
void pickup_chopsticks(int id) {
    pthread_mutex_lock(&mutex);
    
    states[id] = HUNGRY;
    printf("Philosopher %d is hungry\n", id);
    
    while (!can_eat(id)) {
        pthread_cond_wait(&condition[id], &mutex);
    }
    
    states[id] = EATING;
    printf("Philosopher %d is eating\n", id);
    
    pthread_mutex_unlock(&mutex);
}

/**
 * @brief Put down chopsticks
 * 
 * @param id Philosopher ID
 */
void putdown_chopsticks(int id) {
    pthread_mutex_lock(&mutex);
    
    states[id] = THINKING;
    printf("Philosopher %d is thinking\n", id);
    
    // Notify neighbors
    int left = (id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    if (states[left] == HUNGRY && can_eat(left)) {
        pthread_cond_signal(&condition[left]);
    }
    if (states[right] == HUNGRY && can_eat(right)) {
        pthread_cond_signal(&condition[right]);
    }
    
    pthread_mutex_unlock(&mutex);
}

/**
 * @brief Philosopher thread function
 * 
 * @param arg Philosopher ID
 * @return void* Always returns NULL
 */
void* philosopher(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        // Think
        sleep(THINKING_TIME);
        
        // Get hungry and try to eat
        pickup_chopsticks(id);
        
        // Eat
        sleep(EATING_TIME);
        
        // Put down chopsticks
        putdown_chopsticks(id);
    }
    
    return NULL;
}

/**
 * @brief Main function
 */
int main(void) {
    // Initialize mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_init(&condition[i], NULL);
        states[i] = THINKING;
        philosopher_ids[i] = i;
    }
    
    // Create philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_create(&philosophers[i], NULL, philosopher, &philosopher_ids[i]);
    }
    
    // Wait for threads (they won't complete in this example)
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    // Clean up
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_cond_destroy(&condition[i]);
    }
    
    return 0;
}
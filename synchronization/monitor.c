/**
 * @file monitor.c
 * @brief Implementation of a monitor for thread synchronization
 * 
 * This program demonstrates the implementation of a monitor
 * using mutexes and condition variables. It shows how to
 * implement thread-safe operations using monitor pattern.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define BUFFER_SIZE 10

/**
 * @brief Monitor structure for thread-safe buffer operations
 */
typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} monitor_t;

monitor_t monitor;

/**
 * @brief Initialize the monitor
 */
void monitor_init(void) {
    monitor.count = 0;
    monitor.in = 0;
    monitor.out = 0;
    
    pthread_mutex_init(&monitor.mutex, NULL);
    pthread_cond_init(&monitor.not_full, NULL);
    pthread_cond_init(&monitor.not_empty, NULL);
}

/**
 * @brief Insert an item into the buffer
 * 
 * @param item The item to insert
 */
void monitor_insert(int item) {
    pthread_mutex_lock(&monitor.mutex);
    
    // Wait while buffer is full
    while (monitor.count == BUFFER_SIZE) {
        pthread_cond_wait(&monitor.not_full, &monitor.mutex);
    }
    
    // Insert item
    monitor.buffer[monitor.in] = item;
    monitor.in = (monitor.in + 1) % BUFFER_SIZE;
    monitor.count++;
    
    // Signal that buffer is not empty
    pthread_cond_signal(&monitor.not_empty);
    pthread_mutex_unlock(&monitor.mutex);
}

/**
 * @brief Remove an item from the buffer
 * 
 * @return int The removed item
 */
int monitor_remove(void) {
    pthread_mutex_lock(&monitor.mutex);
    
    // Wait while buffer is empty
    while (monitor.count == 0) {
        pthread_cond_wait(&monitor.not_empty, &monitor.mutex);
    }
    
    // Remove item
    int item = monitor.buffer[monitor.out];
    monitor.out = (monitor.out + 1) % BUFFER_SIZE;
    monitor.count--;
    
    // Signal that buffer is not full
    pthread_cond_signal(&monitor.not_full);
    pthread_mutex_unlock(&monitor.mutex);
    
    return item;
}

/**
 * @brief Producer thread function
 */
void* producer(void* arg) {
    int item;
    while (true) {
        // Generate a random item
        item = rand() % 100;
        
        // Insert item into buffer
        monitor_insert(item);
        printf("Producer inserted: %d\n", item);
        
        // Simulate some work
        usleep(100000);
    }
    return NULL;
}

/**
 * @brief Consumer thread function
 */
void* consumer(void* arg) {
    int item;
    while (true) {
        // Remove item from buffer
        item = monitor_remove();
        printf("Consumer removed: %d\n", item);
        
        // Simulate some work
        usleep(150000);
    }
    return NULL;
}

/**
 * @brief Main function demonstrating monitor usage
 */
int main(void) {
    pthread_t producer_thread, consumer_thread;
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize monitor
    monitor_init();
    
    // Create producer and consumer threads
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);
    
    // Wait for threads to complete (they won't in this example)
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    
    return 0;
} 
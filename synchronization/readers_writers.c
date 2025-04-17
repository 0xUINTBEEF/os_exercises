/**
 * @file readers_writers.c
 * @brief Implementation of the readers-writers problem with writer priority
 * 
 * This program demonstrates a solution to the readers-writers problem
 * that prevents writer starvation by giving writers priority over readers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

/**
 * @brief Monitor structure for readers-writers synchronization
 */
typedef struct {
    int readers_count;
    bool writer_active;
    int writers_waiting;
    
    pthread_mutex_t mutex;
    pthread_cond_t readers;
    pthread_cond_t writers;
} rw_monitor_t;

rw_monitor_t monitor;

/**
 * @brief Initialize the readers-writers monitor
 */
void monitor_init(void) {
    monitor.readers_count = 0;
    monitor.writer_active = false;
    monitor.writers_waiting = 0;
    
    pthread_mutex_init(&monitor.mutex, NULL);
    pthread_cond_init(&monitor.readers, NULL);
    pthread_cond_init(&monitor.writers, NULL);
}

/**
 * @brief Start reading operation
 */
void start_read(void) {
    pthread_mutex_lock(&monitor.mutex);
    
    // Wait if there's an active writer or waiting writers
    while (monitor.writer_active || monitor.writers_waiting > 0) {
        pthread_cond_wait(&monitor.readers, &monitor.mutex);
    }
    
    monitor.readers_count++;
    printf("Reader started. Active readers: %d\n", monitor.readers_count);
    
    pthread_mutex_unlock(&monitor.mutex);
}

/**
 * @brief End reading operation
 */
void end_read(void) {
    pthread_mutex_lock(&monitor.mutex);
    
    monitor.readers_count--;
    printf("Reader finished. Active readers: %d\n", monitor.readers_count);
    
    // If last reader, signal waiting writer
    if (monitor.readers_count == 0) {
        pthread_cond_signal(&monitor.writers);
    }
    
    pthread_mutex_unlock(&monitor.mutex);
}

/**
 * @brief Start writing operation
 */
void start_write(void) {
    pthread_mutex_lock(&monitor.mutex);
    
    monitor.writers_waiting++;
    
    // Wait until no readers and no active writer
    while (monitor.readers_count > 0 || monitor.writer_active) {
        pthread_cond_wait(&monitor.writers, &monitor.mutex);
    }
    
    monitor.writers_waiting--;
    monitor.writer_active = true;
    printf("Writer started\n");
    
    pthread_mutex_unlock(&monitor.mutex);
}

/**
 * @brief End writing operation
 */
void end_write(void) {
    pthread_mutex_lock(&monitor.mutex);
    
    monitor.writer_active = false;
    printf("Writer finished\n");
    
    // If no waiting writers, wake up all readers
    if (monitor.writers_waiting == 0) {
        pthread_cond_broadcast(&monitor.readers);
    } else {
        pthread_cond_signal(&monitor.writers);
    }
    
    pthread_mutex_unlock(&monitor.mutex);
}

/**
 * @brief Reader thread function
 */
void* reader(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        // Think for a while
        usleep(rand() % 1000000);
        
        // Read
        start_read();
        printf("Reader %d is reading\n", id);
        usleep(rand() % 500000);  // Simulate reading
        end_read();
    }
    return NULL;
}

/**
 * @brief Writer thread function
 */
void* writer(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        // Think for a while
        usleep(rand() % 1500000);
        
        // Write
        start_write();
        printf("Writer %d is writing\n", id);
        usleep(rand() % 1000000);  // Simulate writing
        end_write();
    }
    return NULL;
}

/**
 * @brief Main function demonstrating readers-writers
 */
int main(void) {
    #define NUM_READERS 5
    #define NUM_WRITERS 2
    
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize monitor
    monitor_init();
    
    // Create reader threads
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }
    
    // Create writer threads
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }
    
    // Wait for threads to complete (they won't in this example)
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    return 0;
}
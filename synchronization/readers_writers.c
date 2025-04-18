/**
 * @file readers_writers.c
 * @brief Implementation of the readers-writers problem
 * 
 * This program demonstrates a solution to the readers-writers problem
 * using mutexes and condition variables. It implements a solution that
 * gives priority to readers (first readers-writers problem).
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_READERS 3
#define NUM_WRITERS 2
#define READING_TIME 2
#define WRITING_TIME 3

// Global variables
pthread_mutex_t mutex;
pthread_cond_t can_read;
pthread_cond_t can_write;
int readers_count = 0;
bool writing = false;
int shared_data = 0;

/**
 * @brief Reader thread function
 * 
 * @param arg Reader ID
 * @return void* Always returns NULL
 */
void* reader(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        // Try to read
        pthread_mutex_lock(&mutex);
        
        // Wait if someone is writing
        while (writing) {
            pthread_cond_wait(&can_read, &mutex);
        }
        
        readers_count++;
        pthread_mutex_unlock(&mutex);
        
        // Read data
        printf("Reader %d is reading data: %d\n", id, shared_data);
        sleep(READING_TIME);
        
        // Done reading
        pthread_mutex_lock(&mutex);
        readers_count--;
        
        // If no readers left, signal writers
        if (readers_count == 0) {
            pthread_cond_signal(&can_write);
        }
        
        pthread_mutex_unlock(&mutex);
        
        // Think for a while
        sleep(1);
    }
    
    return NULL;
}

/**
 * @brief Writer thread function
 * 
 * @param arg Writer ID
 * @return void* Always returns NULL
 */
void* writer(void* arg) {
    int id = *(int*)arg;
    
    while (true) {
        // Try to write
        pthread_mutex_lock(&mutex);
        
        // Wait if someone is writing or reading
        while (writing || readers_count > 0) {
            pthread_cond_wait(&can_write, &mutex);
        }
        
        writing = true;
        pthread_mutex_unlock(&mutex);
        
        // Write data
        shared_data++;
        printf("Writer %d is writing data: %d\n", id, shared_data);
        sleep(WRITING_TIME);
        
        // Done writing
        pthread_mutex_lock(&mutex);
        writing = false;
        
        // Signal waiting threads
        pthread_cond_signal(&can_write);
        pthread_cond_broadcast(&can_read);
        
        pthread_mutex_unlock(&mutex);
        
        // Think for a while
        sleep(1);
    }
    
    return NULL;
}

/**
 * @brief Main function
 */
int main(void) {
    pthread_t readers[NUM_READERS];
    pthread_t writers[NUM_WRITERS];
    int reader_ids[NUM_READERS];
    int writer_ids[NUM_WRITERS];
    
    // Initialize mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&can_read, NULL);
    pthread_cond_init(&can_write, NULL);
    
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
    
    // Wait for threads (they won't complete in this example)
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }
    
    // Clean up
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&can_read);
    pthread_cond_destroy(&can_write);
    
    return 0;
}
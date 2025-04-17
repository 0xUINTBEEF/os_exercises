/**
 * @file thread_pool.c
 * @brief Implementation of a thread pool
 * 
 * This program demonstrates a simple thread pool implementation
 * that can execute tasks in parallel using a fixed number of threads.
 * It includes task queue management and graceful shutdown.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_THREADS 4
#define MAX_TASKS 100

// Task structure
typedef struct {
    void (*function)(void*);
    void* argument;
} task_t;

// Thread pool structure
typedef struct {
    pthread_t threads[MAX_THREADS];
    task_t task_queue[MAX_TASKS];
    int queue_size;
    int queue_front;
    int queue_rear;
    
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
    
    bool shutdown;
} thread_pool_t;

// Global thread pool instance
thread_pool_t pool;

/**
 * @brief Initialize the thread pool
 * 
 * Sets up the thread pool with the specified number of threads
 * and initializes synchronization primitives.
 */
void thread_pool_init(void) {
    pool.queue_size = 0;
    pool.queue_front = 0;
    pool.queue_rear = 0;
    pool.shutdown = false;
    
    pthread_mutex_init(&pool.queue_mutex, NULL);
    pthread_cond_init(&pool.queue_not_empty, NULL);
    pthread_cond_init(&pool.queue_not_full, NULL);
    
    // Create worker threads
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&pool.threads[i], NULL, (void*)worker_thread, NULL);
    }
}

/**
 * @brief Worker thread function
 * 
 * Continuously processes tasks from the queue until shutdown is requested.
 */
void* worker_thread(void* arg) {
    while (true) {
        pthread_mutex_lock(&pool.queue_mutex);
        
        // Wait for tasks or shutdown
        while (pool.queue_size == 0 && !pool.shutdown) {
            pthread_cond_wait(&pool.queue_not_empty, &pool.queue_mutex);
        }
        
        if (pool.shutdown) {
            pthread_mutex_unlock(&pool.queue_mutex);
            pthread_exit(NULL);
        }
        
        // Get task from queue
        task_t task = pool.task_queue[pool.queue_front];
        pool.queue_front = (pool.queue_front + 1) % MAX_TASKS;
        pool.queue_size--;
        
        pthread_cond_signal(&pool.queue_not_full);
        pthread_mutex_unlock(&pool.queue_mutex);
        
        // Execute task
        (task.function)(task.argument);
    }
}

/**
 * @brief Add a task to the thread pool
 * 
 * @param function The function to execute
 * @param argument The argument to pass to the function
 * @return int 0 on success, -1 on failure
 */
int thread_pool_add_task(void (*function)(void*), void* argument) {
    pthread_mutex_lock(&pool.queue_mutex);
    
    while (pool.queue_size == MAX_TASKS && !pool.shutdown) {
        pthread_cond_wait(&pool.queue_not_full, &pool.queue_mutex);
    }
    
    if (pool.shutdown) {
        pthread_mutex_unlock(&pool.queue_mutex);
        return -1;
    }
    
    // Add task to queue
    pool.task_queue[pool.queue_rear].function = function;
    pool.task_queue[pool.queue_rear].argument = argument;
    pool.queue_rear = (pool.queue_rear + 1) % MAX_TASKS;
    pool.queue_size++;
    
    pthread_cond_signal(&pool.queue_not_empty);
    pthread_mutex_unlock(&pool.queue_mutex);
    
    return 0;
}

/**
 * @brief Shutdown the thread pool
 * 
 * Signals all threads to exit and waits for them to complete.
 */
void thread_pool_shutdown(void) {
    pthread_mutex_lock(&pool.queue_mutex);
    pool.shutdown = true;
    pthread_cond_broadcast(&pool.queue_not_empty);
    pthread_mutex_unlock(&pool.queue_mutex);
    
    // Wait for all threads to complete
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool.threads[i], NULL);
    }
    
    // Clean up
    pthread_mutex_destroy(&pool.queue_mutex);
    pthread_cond_destroy(&pool.queue_not_empty);
    pthread_cond_destroy(&pool.queue_not_full);
}

// Example task function
void example_task(void* arg) {
    int* number = (int*)arg;
    printf("Processing task with number: %d\n", *number);
    free(number);
}

/**
 * @brief Main function demonstrating thread pool usage
 */
int main(void) {
    thread_pool_init();
    
    // Add some tasks
    for (int i = 0; i < 10; i++) {
        int* number = malloc(sizeof(int));
        *number = i;
        thread_pool_add_task(example_task, number);
    }
    
    // Wait for tasks to complete
    sleep(2);
    
    thread_pool_shutdown();
    return 0;
} 
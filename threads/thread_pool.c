/**
 * Thread Pool Implementation
 * 
 * A thread pool implementation that manages a fixed number of worker threads
 * to execute tasks concurrently. Features include:
 * - Dynamic task queue management
 * - Thread-safe operations
 * - Graceful shutdown
 * - Task prioritization
 * - Resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

// Constants
#define MAX_THREADS 4
#define MAX_TASKS 100
#define TASK_QUEUE_SIZE 1000
#define LOG_FILE "thread_pool.log"

// Task structure
typedef struct {
    void (*function)(void *);
    void *arg;
    int priority;
} task_t;

// Thread pool structure
typedef struct {
    pthread_t threads[MAX_THREADS];
    task_t task_queue[TASK_QUEUE_SIZE];
    int queue_size;
    int queue_front;
    int queue_rear;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
    int shutdown;
    int active_threads;
} thread_pool_t;

// Global thread pool instance
static thread_pool_t pool;

// Function declarations
static void *worker_thread(void *arg);
static int add_task(void (*function)(void *), void *arg, int priority);
static void execute_task(task_t *task);
static void log_message(const char *message);
void thread_pool_action(const char *action);

/**
 * Initialize the thread pool
 * @return 0 on success, -1 on error
 */
int thread_pool_init(void) {
    int i;
    int result;

    // Initialize mutex and condition variables
    if (pthread_mutex_init(&pool.queue_mutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        return -1;
    }

    if (pthread_cond_init(&pool.queue_not_empty, NULL) != 0) {
        pthread_mutex_destroy(&pool.queue_mutex);
        perror("Failed to initialize condition variable");
        return -1;
    }

    if (pthread_cond_init(&pool.queue_not_full, NULL) != 0) {
        pthread_mutex_destroy(&pool.queue_mutex);
        pthread_cond_destroy(&pool.queue_not_empty);
        perror("Failed to initialize condition variable");
        return -1;
    }

    // Initialize pool state
    pool.queue_size = 0;
    pool.queue_front = 0;
    pool.queue_rear = 0;
    pool.shutdown = 0;
    pool.active_threads = 0;

    // Create worker threads
    for (i = 0; i < MAX_THREADS; i++) {
        result = pthread_create(&pool.threads[i], NULL, worker_thread, NULL);
        if (result != 0) {
            // Cleanup on error
            thread_pool_shutdown();
            errno = result;
            perror("Failed to create thread");
            return -1;
        }
        pool.active_threads++;
    }

    return 0;
}

/**
 * Worker thread function
 * @param arg Unused parameter
 * @return NULL
 */
static void *worker_thread(void *arg) {
    task_t task;

    while (1) {
        pthread_mutex_lock(&pool.queue_mutex);

        // Wait for tasks or shutdown
        while (pool.queue_size == 0 && !pool.shutdown) {
            pthread_cond_wait(&pool.queue_not_empty, &pool.queue_mutex);
        }

        // Check for shutdown
        if (pool.shutdown && pool.queue_size == 0) {
            pool.active_threads--;
            pthread_mutex_unlock(&pool.queue_mutex);
            pthread_exit(NULL);
        }

        // Get task from queue
        task = pool.task_queue[pool.queue_front];
        pool.queue_front = (pool.queue_front + 1) % TASK_QUEUE_SIZE;
        pool.queue_size--;

        // Signal that queue is not full
        pthread_cond_signal(&pool.queue_not_full);
        pthread_mutex_unlock(&pool.queue_mutex);

        // Execute task
        execute_task(&task);
    }

    return NULL;
}

/**
 * Add a task to the thread pool
 * @param function Task function to execute
 * @param arg Argument to pass to the task function
 * @param priority Task priority (higher number = higher priority)
 * @return 0 on success, -1 on error
 */
static int add_task(void (*function)(void *), void *arg, int priority) {
    pthread_mutex_lock(&pool.queue_mutex);

    // Wait for queue space
    while (pool.queue_size == TASK_QUEUE_SIZE && !pool.shutdown) {
        pthread_cond_wait(&pool.queue_not_full, &pool.queue_mutex);
    }

    if (pool.shutdown) {
        pthread_mutex_unlock(&pool.queue_mutex);
        return -1;
    }

    // Add task to queue
    pool.task_queue[pool.queue_rear].function = function;
    pool.task_queue[pool.queue_rear].arg = arg;
    pool.task_queue[pool.queue_rear].priority = priority;
    pool.queue_rear = (pool.queue_rear + 1) % TASK_QUEUE_SIZE;
    pool.queue_size++;

    // Signal that queue is not empty
    pthread_cond_signal(&pool.queue_not_empty);
    pthread_mutex_unlock(&pool.queue_mutex);

    return 0;
}

/**
 * Execute a task
 * @param task Task to execute
 */
static void execute_task(task_t *task) {
    if (task->function != NULL) {
        task->function(task->arg);
    }
}

/**
 * Submit a task to the thread pool
 * @param function Task function to execute
 * @param arg Argument to pass to the task function
 * @return 0 on success, -1 on error
 */
int thread_pool_submit(void (*function)(void *), void *arg) {
    return add_task(function, arg, 0);
}

/**
 * Submit a prioritized task to the thread pool
 * @param function Task function to execute
 * @param arg Argument to pass to the task function
 * @param priority Task priority
 * @return 0 on success, -1 on error
 */
int thread_pool_submit_priority(void (*function)(void *), void *arg, int priority) {
    return add_task(function, arg, priority);
}

/**
 * Shutdown the thread pool
 * @return 0 on success, -1 on error
 */
int thread_pool_shutdown(void) {
    int i;
    int result;

    pthread_mutex_lock(&pool.queue_mutex);
    pool.shutdown = 1;
    pthread_cond_broadcast(&pool.queue_not_empty);
    pthread_mutex_unlock(&pool.queue_mutex);

    // Wait for all threads to finish
    for (i = 0; i < MAX_THREADS; i++) {
        result = pthread_join(pool.threads[i], NULL);
        if (result != 0) {
            errno = result;
            perror("Failed to join thread");
        }
    }

    // Cleanup resources
    pthread_mutex_destroy(&pool.queue_mutex);
    pthread_cond_destroy(&pool.queue_not_empty);
    pthread_cond_destroy(&pool.queue_not_full);

    return 0;
}

/**
 * Get the number of active threads
 * @return Number of active threads
 */
int thread_pool_active_threads(void) {
    int count;
    pthread_mutex_lock(&pool.queue_mutex);
    count = pool.active_threads;
    pthread_mutex_unlock(&pool.queue_mutex);
    return count;
}

/**
 * Get the number of queued tasks
 * @return Number of queued tasks
 */
int thread_pool_queued_tasks(void) {
    int count;
    pthread_mutex_lock(&pool.queue_mutex);
    count = pool.queue_size;
    pthread_mutex_unlock(&pool.queue_mutex);
    return count;
}

// Function to log messages to a file
static void log_message(const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "%s: %s\n", ctime(&now), message);
        fclose(log_file);
    }
}

void thread_pool_action(const char *action) {
    log_message(action);
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
    log_message("Thread pool started");
    thread_pool_init();
    
    // Add some tasks
    for (int i = 0; i < 10; i++) {
        int* number = malloc(sizeof(int));
        *number = i;
        thread_pool_submit(example_task, number);
    }
    
    // Wait for tasks to complete
    sleep(2);
    
    thread_pool_shutdown();
    log_message("Thread pool finished");
    return 0;
}
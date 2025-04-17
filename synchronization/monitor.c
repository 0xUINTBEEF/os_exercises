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
typedef enum {
    MONITOR_SUCCESS = 0,
    MONITOR_ERROR_MUTEX_INIT = -1,
    MONITOR_ERROR_COND_INIT = -2,
    MONITOR_ERROR_MUTEX_LOCK = -3,
    MONITOR_ERROR_NULL_POINTER = -4,
    MONITOR_ERROR_TIMEOUT = -5
} monitor_error_t;

typedef struct thread_info {
    pthread_t id;
    time_t last_active;
    bool is_waiting;
    int priority;           // Thread priority
    int inherited_priority; // Inherited priority for priority inheritance
    struct thread_info* next;
} thread_info_t;

typedef struct {
    // Buffer state
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    bool initialized;
    
    // Thread monitoring
    thread_info_t* waiting_threads;
    int waiting_count;
    time_t deadlock_threshold;
    thread_info_t* owner;   // Current owner of the monitor
    
    // Performance metrics
    unsigned long total_insertions;
    unsigned long total_removals;
    unsigned long total_timeouts;
    double avg_wait_time;
    struct timespec start_time;
    
    // Priority inheritance metrics
    unsigned long priority_inversions;
    unsigned long priority_inheritances;
    
    // Synchronization primitives
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} monitor_t;

monitor_t monitor;

/**
 * @brief Initialize the monitor
 */
monitor_error_t monitor_init(void) {
    // Initialize buffer state
    monitor.count = 0;
    monitor.in = 0;
    monitor.out = 0;
    monitor.initialized = false;
    
    // Initialize thread monitoring
    monitor.waiting_threads = NULL;
    monitor.waiting_count = 0;
    monitor.deadlock_threshold = 5;
    
    // Initialize performance metrics
    monitor.total_insertions = 0;
    monitor.total_removals = 0;
    monitor.total_timeouts = 0;
    monitor.avg_wait_time = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &monitor.start_time);
    
    if (pthread_mutex_init(&monitor.mutex, NULL) != 0) {
        return MONITOR_ERROR_MUTEX_INIT;
    }
    
    if (pthread_cond_init(&monitor.not_full, NULL) != 0) {
        pthread_mutex_destroy(&monitor.mutex);
        return MONITOR_ERROR_COND_INIT;
    }
    
    if (pthread_cond_init(&monitor.not_empty, NULL) != 0) {
        pthread_mutex_destroy(&monitor.mutex);
        pthread_cond_destroy(&monitor.not_full);
        return MONITOR_ERROR_COND_INIT;
    }
    
    monitor.initialized = true;
    return MONITOR_SUCCESS;
}

/**
 * @brief Insert an item into the buffer with timeout
 * 
 * @param item The item to insert
 * @param timeout_ms Timeout in milliseconds, 0 for no timeout
 * @return int 0 on success, -1 on timeout or error
 */
monitor_error_t monitor_insert(int item, unsigned int timeout_ms) {
    if (!monitor.initialized) {
        return MONITOR_ERROR_MUTEX_INIT;
    }

    struct timespec ts;
    int ret = 0;

    if (pthread_mutex_lock(&monitor.mutex) != 0) {
        return MONITOR_ERROR_MUTEX_LOCK;
    }
    
    add_waiting_thread();
    
    // Wait while buffer is full
    while (monitor.count == BUFFER_SIZE && ret == 0) {
        check_deadlock();
        if (timeout_ms > 0) {
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout_ms / 1000;
            ts.tv_nsec += (timeout_ms % 1000) * 1000000;
            ret = pthread_cond_timedwait(&monitor.not_full, &monitor.mutex, &ts);
        } else {
            ret = pthread_cond_wait(&monitor.not_full, &monitor.mutex);
        }
    }
    
    remove_waiting_thread();
    
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double wait_time = (end_time.tv_sec - ts.tv_sec) + 
                      (end_time.tv_nsec - ts.tv_nsec) / 1e9;
    
    if (ret == 0) {
        // Insert item
        monitor.buffer[monitor.in] = item;
        monitor.in = (monitor.in + 1) % BUFFER_SIZE;
        monitor.count++;
        monitor.total_insertions++;
        
        // Update performance metrics
        monitor.avg_wait_time = (monitor.avg_wait_time * 
            (monitor.total_insertions - 1) + wait_time) / 
            monitor.total_insertions;
        
        // Signal that buffer is not empty
        pthread_cond_signal(&monitor.not_empty);
    } else {
        monitor.total_timeouts++;
    }
    
    pthread_mutex_unlock(&monitor.mutex);
    return (ret == 0) ? MONITOR_SUCCESS : MONITOR_ERROR_TIMEOUT;
}

/**
 * @brief Remove an item from the buffer with timeout
 * 
 * @param item Pointer to store the removed item
 * @param timeout_ms Timeout in milliseconds, 0 for no timeout
 * @return int 0 on success, -1 on timeout or error
 */
monitor_error_t monitor_remove(int* item, unsigned int timeout_ms) {
    if (!monitor.initialized) {
        return MONITOR_ERROR_MUTEX_INIT;
    }

    if (item == NULL) {
        return MONITOR_ERROR_NULL_POINTER;
    }

    struct timespec ts;
    int ret = 0;

    if (pthread_mutex_lock(&monitor.mutex) != 0) {
        return MONITOR_ERROR_MUTEX_LOCK;
    }
    
    add_waiting_thread();
    
    // Wait while buffer is empty
    while (monitor.count == 0 && ret == 0) {
        check_deadlock();
        if (timeout_ms > 0) {
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += timeout_ms / 1000;
            ts.tv_nsec += (timeout_ms % 1000) * 1000000;
            ret = pthread_cond_timedwait(&monitor.not_empty, &monitor.mutex, &ts);
        } else {
            ret = pthread_cond_wait(&monitor.not_empty, &monitor.mutex);
        }
    }
    
    remove_waiting_thread();
    
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double wait_time = (end_time.tv_sec - ts.tv_sec) + 
                      (end_time.tv_nsec - ts.tv_nsec) / 1e9;
    
    if (ret == 0) {
        // Remove item
        *item = monitor.buffer[monitor.out];
        monitor.out = (monitor.out + 1) % BUFFER_SIZE;
        monitor.count--;
        monitor.total_removals++;
        
        // Update performance metrics
        monitor.avg_wait_time = (monitor.avg_wait_time * 
            (monitor.total_removals - 1) + wait_time) / 
            monitor.total_removals;
        
        // Signal that buffer is not full
        pthread_cond_signal(&monitor.not_full);
    } else {
        monitor.total_timeouts++;
    }
    
    pthread_mutex_unlock(&monitor.mutex);
    return (ret == 0) ? MONITOR_SUCCESS : MONITOR_ERROR_TIMEOUT;
}

/**
 * @brief Producer thread function
 */
void* producer(void* arg) {
    int item;
    while (true) {
        // Generate a random item
        item = rand() % 100;
        
        // Insert item into buffer with 1 second timeout
        if (monitor_insert(item, 1000) == 0) {
            printf("Producer inserted: %d\n", item);
        } else {
            printf("Producer timeout or error\n");
        }
        
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
        // Remove item from buffer with 1.5 second timeout
        if (monitor_remove(&item, 1500) == 0) {
            printf("Consumer removed: %d\n", item);
        } else {
            printf("Consumer timeout or error\n");
        }
        
        // Simulate some work
        usleep(150000);
    }
    return NULL;
}

/**
 * @brief Main function demonstrating monitor usage
 */
void check_deadlock(void) {
    thread_info_t* current = monitor.waiting_threads;
    time_t now = time(NULL);
    
    while (current != NULL) {
        if (current->is_waiting && (now - current->last_active) > monitor.deadlock_threshold) {
            printf("WARNING: Potential deadlock detected! Thread %lu has been waiting for %ld seconds\n", 
                   (unsigned long)current->id, (now - current->last_active));
        }
        current = current->next;
    }
}

void add_waiting_thread(void) {
    thread_info_t* new_thread = malloc(sizeof(thread_info_t));
    new_thread->id = pthread_self();
    new_thread->last_active = time(NULL);
    new_thread->is_waiting = true;
    new_thread->priority = 0;  // Default priority
    new_thread->inherited_priority = 0;
    new_thread->next = monitor.waiting_threads;
    
    // Check for priority inheritance
    if (monitor.owner != NULL && new_thread->priority > monitor.owner->priority) {
        monitor.owner->inherited_priority = new_thread->priority;
        monitor.priority_inheritances++;
        printf("Priority inheritance: Thread %lu inherited priority %d from Thread %lu\n",
               (unsigned long)monitor.owner->id, new_thread->priority,
               (unsigned long)new_thread->id);
    }
    
    monitor.waiting_threads = new_thread;
    monitor.waiting_count++;
}

void remove_waiting_thread(void) {
    pthread_t current_id = pthread_self();
    thread_info_t* current = monitor.waiting_threads;
    thread_info_t* prev = NULL;
    
    while (current != NULL) {
        if (pthread_equal(current->id, current_id)) {
            if (prev == NULL) {
                monitor.waiting_threads = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            monitor.waiting_count--;
            break;
        }
        prev = current;
        current = current->next;
    }
}

void print_monitor_stats(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double total_time = (now.tv_sec - monitor.start_time.tv_sec) + 
                       (now.tv_nsec - monitor.start_time.tv_nsec) / 1e9;
    
    printf("\nMonitor Statistics:\n");
    printf("Total runtime: %.2f seconds\n", total_time);
    printf("Total insertions: %lu\n", monitor.total_insertions);
    printf("Total removals: %lu\n", monitor.total_removals);
    printf("Total timeouts: %lu\n", monitor.total_timeouts);
    printf("Average wait time: %.6f seconds\n", monitor.avg_wait_time);
    printf("Operations per second: %.2f\n", 
           (monitor.total_insertions + monitor.total_removals) / total_time);
}

void monitor_destroy(void) {
    if (!monitor.initialized) {
        return;
    }
    
    // Print final statistics
    print_monitor_stats();
    
    // Clean up waiting threads list
    thread_info_t* current = monitor.waiting_threads;
    while (current != NULL) {
        thread_info_t* next = current->next;
        free(current);
        current = next;
    }
    
    pthread_mutex_destroy(&monitor.mutex);
    pthread_cond_destroy(&monitor.not_full);
    pthread_cond_destroy(&monitor.not_empty);
    monitor.initialized = false;
}

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
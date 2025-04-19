/**
 * @file thread_scheduler.c
 * @brief Thread scheduling visualization implementation
 * 
 * This program demonstrates thread scheduling visualization
 * with real-time display of thread states and priority-based
 * scheduling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>

#define NUM_THREADS 5
#define MAX_PRIORITY 10
#define SCHEDULING_INTERVAL 1

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ThreadState;

typedef struct {
    pthread_t tid;
    int id;
    int priority;
    ThreadState state;
    int cpu_time;
    int total_time;
    pthread_mutex_t mutex;
} ThreadInfo;

ThreadInfo threads[NUM_THREADS];
pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Initialize thread information
 */
void init_threads(void) {
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].id = i;
        threads[i].priority = rand() % MAX_PRIORITY + 1;
        threads[i].state = READY;
        threads[i].cpu_time = 0;
        threads[i].total_time = 0;
        pthread_mutex_init(&threads[i].mutex, NULL);
    }
}

/**
 * @brief Display thread states
 */
void display_threads(void) {
    pthread_mutex_lock(&display_mutex);
    
    clear();
    printw("Thread Scheduling Visualization\n");
    printw("==============================\n\n");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_mutex_lock(&threads[i].mutex);
        
        printw("Thread %d:\n", threads[i].id);
        printw("  Priority: %d\n", threads[i].priority);
        printw("  State: ");
        switch (threads[i].state) {
            case READY: printw("READY"); break;
            case RUNNING: printw("RUNNING"); break;
            case BLOCKED: printw("BLOCKED"); break;
            case TERMINATED: printw("TERMINATED"); break;
        }
        printw("\n");
        printw("  CPU Time: %d\n", threads[i].cpu_time);
        printw("  Total Time: %d\n", threads[i].total_time);
        printw("\n");
        
        pthread_mutex_unlock(&threads[i].mutex);
    }
    
    refresh();
    pthread_mutex_unlock(&display_mutex);
}

/**
 * @brief Thread function
 */
void* thread_function(void* arg) {
    ThreadInfo* info = (ThreadInfo*)arg;
    
    while (info->state != TERMINATED) {
        pthread_mutex_lock(&info->mutex);
        
        if (info->state == RUNNING) {
            info->cpu_time++;
            info->total_time++;
            
            // Simulate work
            usleep(100000);
            
            // Random state change
            if (rand() % 10 == 0) {
                info->state = BLOCKED;
            }
        } else if (info->state == BLOCKED) {
            info->total_time++;
            
            // Simulate I/O
            usleep(200000);
            
            // Return to ready state
            info->state = READY;
        }
        
        pthread_mutex_unlock(&info->mutex);
        usleep(10000); // Prevent CPU hogging
    }
    
    return NULL;
}

/**
 * @brief Scheduler function
 */
void* scheduler_function(void* arg) {
    (void)arg;
    
    while (1) {
        // Find highest priority ready thread
        int highest_priority = 0;
        int selected_thread = -1;
        
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_mutex_lock(&threads[i].mutex);
            
            if (threads[i].state == READY && 
                threads[i].priority > highest_priority) {
                highest_priority = threads[i].priority;
                selected_thread = i;
            }
            
            pthread_mutex_unlock(&threads[i].mutex);
        }
        
        // Schedule selected thread
        if (selected_thread != -1) {
            pthread_mutex_lock(&threads[selected_thread].mutex);
            threads[selected_thread].state = RUNNING;
            pthread_mutex_unlock(&threads[selected_thread].mutex);
        }
        
        // Display current state
        display_threads();
        
        // Sleep for scheduling interval
        sleep(SCHEDULING_INTERVAL);
    }
    
    return NULL;
}

int main(void) {
    pthread_t scheduler_thread;
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize threads
    init_threads();
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i].tid, NULL, thread_function, &threads[i]);
    }
    
    // Create scheduler thread
    pthread_create(&scheduler_thread, NULL, scheduler_function, NULL);
    
    // Wait for user input to exit
    getch();
    
    // Cleanup
    endwin();
    
    // Terminate threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_mutex_lock(&threads[i].mutex);
        threads[i].state = TERMINATED;
        pthread_mutex_unlock(&threads[i].mutex);
        pthread_join(threads[i].tid, NULL);
        pthread_mutex_destroy(&threads[i].mutex);
    }
    
    pthread_mutex_destroy(&display_mutex);
    
    return 0;
} 
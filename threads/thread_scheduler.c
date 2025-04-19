/**
 * Thread Scheduling Visualization using POSIX Threads and ncurses
 * 
 * This program demonstrates thread scheduling with real-time visualization
 * using POSIX threads and ncurses library. Features include:
 * - Real-time display of thread states
 * - Priority-based scheduling
 * - Thread state tracking
 * - Performance monitoring
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

// Constants
#define MAX_THREADS 5
#define MAX_ITERATIONS 1000000
#define PRIORITY_LEVELS 5
#define REFRESH_RATE 100000  // microseconds

// Thread states
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} thread_state_t;

// Structure to hold thread information
typedef struct {
    int id;
    int priority;
    thread_state_t state;
    unsigned long long cpu_time;
    unsigned long long total_time;
    struct timespec start_time;
    struct timespec end_time;
} thread_info_t;

// Global variables
static pthread_mutex_t scheduler_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t scheduler_cond = PTHREAD_COND_INITIALIZER;
static thread_info_t thread_info[MAX_THREADS];
static int running_thread = -1;
static int should_exit = 0;

/**
 * Initialize thread information
 */
static void init_threads(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_info[i].id = i;
        thread_info[i].priority = (i % PRIORITY_LEVELS) + 1;
        thread_info[i].state = READY;
        thread_info[i].cpu_time = 0;
        thread_info[i].total_time = 0;
    }
}

/**
 * Display thread states and information
 */
static void display_threads(void) {
    clear();
    mvprintw(0, 0, "Thread Scheduler Visualization");
    mvprintw(1, 0, "Press 'q' to quit");
    mvprintw(3, 0, "Thread ID  Priority  State      CPU Time    Total Time");
    mvprintw(4, 0, "------------------------------------------------------");

    for (int i = 0; i < MAX_THREADS; i++) {
        const char *state_str;
        switch (thread_info[i].state) {
            case READY: state_str = "READY"; break;
            case RUNNING: state_str = "RUNNING"; break;
            case BLOCKED: state_str = "BLOCKED"; break;
            case TERMINATED: state_str = "TERMINATED"; break;
            default: state_str = "UNKNOWN"; break;
        }

        mvprintw(5 + i, 0, "%-9d %-9d %-10s %-11llu %-11llu",
                thread_info[i].id,
                thread_info[i].priority,
                state_str,
                thread_info[i].cpu_time,
                thread_info[i].total_time);
    }

    refresh();
}

/**
 * Thread function that performs work based on priority
 * @param arg Thread information structure
 * @return NULL
 */
static void *thread_function(void *arg) {
    thread_info_t *info = (thread_info_t *)arg;
    struct timespec start, end;
    double dummy = 0.0;

    while (!should_exit) {
        pthread_mutex_lock(&scheduler_mutex);
        
        // Wait until this thread is selected to run
        while (running_thread != info->id && !should_exit) {
            pthread_cond_wait(&scheduler_cond, &scheduler_mutex);
        }

        if (should_exit) {
            pthread_mutex_unlock(&scheduler_mutex);
            break;
        }

        // Update thread state
        info->state = RUNNING;
        clock_gettime(CLOCK_MONOTONIC, &start);

        pthread_mutex_unlock(&scheduler_mutex);

        // Perform work
        for (int i = 0; i < info->priority * 1000; i++) {
            dummy += (double)i / (i + 1);
        }

        pthread_mutex_lock(&scheduler_mutex);
        
        // Update timing information
        clock_gettime(CLOCK_MONOTONIC, &end);
        info->cpu_time += (end.tv_sec - start.tv_sec) * 1000000000 + 
                         (end.tv_nsec - start.tv_nsec);
        info->total_time = info->cpu_time;
        
        // Change state back to ready
        info->state = READY;
        running_thread = -1;
        
        pthread_cond_broadcast(&scheduler_cond);
        pthread_mutex_unlock(&scheduler_mutex);
    }

    return NULL;
}

/**
 * Scheduler function that selects the next thread to run
 */
static void *scheduler_function(void *arg) {
    (void)arg;  // Unused parameter
    
    while (!should_exit) {
        pthread_mutex_lock(&scheduler_mutex);
        
        // Find highest priority ready thread
        int highest_priority = -1;
        int selected_thread = -1;
        
        for (int i = 0; i < MAX_THREADS; i++) {
            if (thread_info[i].state == READY && 
                thread_info[i].priority > highest_priority) {
                highest_priority = thread_info[i].priority;
                selected_thread = i;
            }
        }
        
        if (selected_thread != -1) {
            running_thread = selected_thread;
            pthread_cond_broadcast(&scheduler_cond);
        }
        
        pthread_mutex_unlock(&scheduler_mutex);
        usleep(REFRESH_RATE);
    }
    
    return NULL;
}

/**
 * Main function demonstrating thread scheduling visualization
 * @return 0 on success, 1 on error
 */
int main(void) {
    pthread_t threads[MAX_THREADS];
    pthread_t scheduler_thread;
    int result;
    int ch;

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    // Initialize thread information
    init_threads();

    // Create scheduler thread
    result = pthread_create(&scheduler_thread, NULL, scheduler_function, NULL);
    if (result != 0) {
        endwin();
        fprintf(stderr, "Error creating scheduler thread: %s\n", strerror(result));
        return 1;
    }

    // Create worker threads
    for (int i = 0; i < MAX_THREADS; i++) {
        result = pthread_create(&threads[i], NULL, thread_function, &thread_info[i]);
        if (result != 0) {
            should_exit = 1;
            pthread_cond_broadcast(&scheduler_cond);
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            pthread_join(scheduler_thread, NULL);
            endwin();
            fprintf(stderr, "Error creating thread %d: %s\n", i, strerror(result));
            return 1;
        }
    }

    // Main loop
    while ((ch = getch()) != 'q') {
        display_threads();
        usleep(REFRESH_RATE);
    }

    // Cleanup
    should_exit = 1;
    pthread_cond_broadcast(&scheduler_cond);
    
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_join(scheduler_thread, NULL);
    endwin();

    return 0;
} 
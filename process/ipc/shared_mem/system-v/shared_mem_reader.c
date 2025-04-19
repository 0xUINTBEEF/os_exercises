/**
 * System V Shared Memory Reader
 * 
 * This program demonstrates reading from shared memory using System V IPC.
 * It attaches to an existing shared memory segment, reads data from it, and properly cleans up resources.
 * 
 * Features:
 * - Shared memory access and management
 * - Error handling
 * - Resource cleanup
 * - Safe memory operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

// Constants
#define SHM_SIZE 1024
#define SHM_KEY_FILE "shared_mem"
#define SHM_KEY_ID 65

// Global variables for cleanup
static int shm_id = -1;
static void *shm_ptr = (void *)-1;
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Cleanup function
static void cleanup(void) {
    if (shm_ptr != (void *)-1) {
        shmdt(shm_ptr);
    }
}

int main(void) {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Generate IPC key
    key_t key = ftok(SHM_KEY_FILE, SHM_KEY_ID);
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    // Get shared memory segment
    shm_id = shmget(key, SHM_SIZE, 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory
    shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Reading from shared memory...\n");

    // Read data from shared memory
    int *data = (int *)shm_ptr;
    printf("Data read from shared memory:\n");
    for (int i = 0; i < 10 && i < SHM_SIZE / sizeof(int); i++) {
        printf("data[%d] = %d\n", i, data[i]);
    }
    printf("...\n");

    // Verify data
    int error = 0;
    for (int i = 0; i < SHM_SIZE / sizeof(int) && running; i++) {
        if (data[i] != i) {
            printf("Data verification failed at index %d: expected %d, got %d\n",
                   i, i, data[i]);
            error = 1;
            break;
        }
    }

    if (!error) {
        printf("Data verification successful\n");
    }

    cleanup();
    printf("Reader process finished\n");
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
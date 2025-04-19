/**
 * System V Shared Memory Writer
 * 
 * This program demonstrates writing to shared memory using System V IPC.
 * It creates a shared memory segment, writes data to it, and properly cleans up resources.
 * 
 * Features:
 * - Shared memory creation and management
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
    if (shm_id != -1) {
        shmctl(shm_id, IPC_RMID, NULL);
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

    // Create shared memory segment
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory created with ID: %d\n", shm_id);

    // Attach shared memory
    shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Writing to shared memory...\n");

    // Write data to shared memory
    int *data = (int *)shm_ptr;
    for (int i = 0; i < SHM_SIZE / sizeof(int) && running; i++) {
        data[i] = i;
    }

    printf("Data written to shared memory:\n");
    for (int i = 0; i < 10 && i < SHM_SIZE / sizeof(int); i++) {
        printf("data[%d] = %d\n", i, data[i]);
    }
    printf("...\n");

    // Wait for reader to finish
    printf("Waiting for reader to finish...\n");
    while (running) {
        sleep(1);
    }

    cleanup();
    printf("Writer process finished\n");
    return EXIT_SUCCESS;
}
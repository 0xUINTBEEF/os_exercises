/**
 * POSIX Shared Memory Writer
 * 
 * This program demonstrates writing to shared memory using POSIX shared memory API.
 * It creates a shared memory object, writes data to it, and properly cleans up resources.
 * 
 * Features:
 * - Shared memory creation and management
 * - Error handling
 * - Resource cleanup
 * - Safe memory operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

// Constants
#define SHM_NAME "/shared_mem"
#define SHM_SIZE 1024
#define NUM_ELEMENTS (SHM_SIZE / sizeof(int))

// Global variables for cleanup
static int shm_fd = -1;
static void *shm_ptr = MAP_FAILED;
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Cleanup function
static void cleanup(void) {
    if (shm_ptr != MAP_FAILED) {
        munmap(shm_ptr, SHM_SIZE);
    }
    if (shm_fd != -1) {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }
}

int main(void) {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Create shared memory object
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Set size of shared memory object
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Map shared memory object
    shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    printf("Writing to shared memory...\n");

    // Write data to shared memory
    int *data = (int *)shm_ptr;
    for (int i = 0; i < NUM_ELEMENTS && running; i++) {
        data[i] = i;
    }

    printf("Data written to shared memory:\n");
    for (int i = 0; i < 10 && i < NUM_ELEMENTS; i++) {
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
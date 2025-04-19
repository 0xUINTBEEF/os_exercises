/**
 * Shared Memory Timer
 * 
 * This program measures the execution time of a command using shared memory
 * for inter-process communication between parent and child processes.
 * 
 * Features:
 * - Command execution timing
 * - Shared memory IPC
 * - Error handling
 * - Resource cleanup
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

// Constants
#define MEM_SIZE sizeof(struct timespec)

// Global variables for cleanup
static void *start_time = MAP_FAILED;
static void *end_time = MAP_FAILED;
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Cleanup function
static void cleanup(void) {
    if (start_time != MAP_FAILED) {
        munmap(start_time, MEM_SIZE);
    }
    if (end_time != MAP_FAILED) {
        munmap(end_time, MEM_SIZE);
    }
}

int main(int argc, char *argv[]) {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Check command line arguments
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Map shared memory for timing
    start_time = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, 
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    end_time = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, 
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (start_time == MAP_FAILED || end_time == MAP_FAILED) {
        perror("Memory mapping failed");
        cleanup();
        return EXIT_FAILURE;
    }

    // Create child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        cleanup();
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child process: execute command
        clock_gettime(CLOCK_REALTIME, (struct timespec *)start_time);

        // Calculate total command length
        int total_length = 0;
        for (int i = 1; i < argc; i++) {
            total_length += strlen(argv[i]) + 1;
        }

        // Allocate and build command string
        char *command = malloc(total_length);
        if (!command) {
            perror("Memory allocation failed");
            cleanup();
            return EXIT_FAILURE;
        }

        strcpy(command, argv[1]);
        for (int i = 2; i < argc; i++) {
            strcat(command, " ");
            strcat(command, argv[i]);
        }

        // Execute command
        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl failed");
        free(command);
        cleanup();
        return EXIT_FAILURE;
    } else {
        // Parent process: wait and measure time
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            clock_gettime(CLOCK_REALTIME, (struct timespec *)end_time);
            
            struct timespec *start = (struct timespec *)start_time;
            struct timespec *end = (struct timespec *)end_time;
            
            double time_taken = (end->tv_sec - start->tv_sec) +
                              (end->tv_nsec - start->tv_nsec) / 1e9;
            
            printf("Command execution time: %.6f seconds\n", time_taken);
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Command terminated by signal %d\n", WTERMSIG(status));
        }
    }

    cleanup();
    return EXIT_SUCCESS;
}
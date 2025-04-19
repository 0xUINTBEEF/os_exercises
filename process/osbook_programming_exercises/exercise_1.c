/**
 * Zombie Process Demonstration
 * 
 * This program demonstrates how zombie processes are created and
 * how they can be prevented using proper process management.
 * 
 * Features:
 * - Zombie process creation
 * - Process state monitoring
 * - Error handling
 * - Resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

// Constants
#define ZOMBIE_LIFETIME 10

// Global variables for cleanup
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("Parent process (PID: %d) starting...\n", getpid());

    // Create child process
    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        // Child process
        printf("Child process (PID: %d) created\n", getpid());
        printf("Child process will terminate in %d seconds\n", ZOMBIE_LIFETIME);
        sleep(ZOMBIE_LIFETIME);
        printf("Child process terminating\n");
        return EXIT_SUCCESS;
    } else {
        // Parent process
        printf("Parent process created child with PID: %d\n", child_pid);
        printf("Child process will become a zombie for %d seconds\n", ZOMBIE_LIFETIME);
        
        // Sleep to allow child to become zombie
        sleep(ZOMBIE_LIFETIME);
        
        // Wait for child to prevent zombie
        int status;
        pid_t waited_pid = waitpid(child_pid, &status, 0);
        
        if (waited_pid == -1) {
            perror("waitpid failed");
            return EXIT_FAILURE;
        }
        
        if (WIFEXITED(status)) {
            printf("Child process %d exited with status %d\n", 
                   waited_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process %d terminated by signal %d\n", 
                   waited_pid, WTERMSIG(status));
        }
    }

    printf("Parent process terminating\n");
    return EXIT_SUCCESS;
}
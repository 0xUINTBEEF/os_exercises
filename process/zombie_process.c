/**
 * Zombie Process Management
 * 
 * This program demonstrates how zombie processes are created and
 * how to prevent them using proper process management techniques.
 * 
 * Features:
 * - Zombie process creation
 * - Process state monitoring
 * - Signal handling
 * - Error handling
 * - Resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>

// Constants
#define NUM_CHILDREN 3
#define CHILD_LIFETIME 2
#define ZOMBIE_LIFETIME 5

// Global variables
static volatile sig_atomic_t running = 1;
static pid_t children[NUM_CHILDREN];

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// SIGCHLD handler to prevent zombies
static void sigchld_handler(int sig) {
    (void)sig;
    int status;
    pid_t pid;
    
    // Wait for any child process
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Child %d exited with status %d\n", 
                   pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child %d killed by signal %d\n", 
                   pid, WTERMSIG(status));
        }
    }
}

// Child process function
static int child_process(int id) {
    printf("Child %d (PID: %d) started\n", id, getpid());
    
    // Simulate some work
    sleep(CHILD_LIFETIME);
    
    printf("Child %d completed\n", id);
    return id;
}

int main(void) {
    // Set up signal handlers
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction failed");
        return EXIT_FAILURE;
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction failed");
        return EXIT_FAILURE;
    }
    
    // Set up SIGCHLD handler
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction failed");
        return EXIT_FAILURE;
    }
    
    printf("Parent process (PID: %d) starting...\n", getpid());
    
    // Create child processes
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork failed");
            return EXIT_FAILURE;
        } else if (pid == 0) {
            // Child process
            return child_process(i);
        } else {
            // Parent process
            children[i] = pid;
            printf("Created child process %d with PID: %d\n", i, pid);
        }
    }
    
    // Parent process continues
    printf("Parent process waiting for children...\n");
    
    // Sleep to allow children to become zombies
    printf("Children will become zombies for %d seconds\n", ZOMBIE_LIFETIME);
    sleep(ZOMBIE_LIFETIME);
    
    // Wait for all children to complete
    for (int i = 0; i < NUM_CHILDREN; i++) {
        int status;
        pid_t pid = waitpid(children[i], &status, 0);
        
        if (pid == -1) {
            perror("waitpid failed");
        } else if (WIFEXITED(status)) {
            printf("Child %d exited normally with status %d\n", 
                   pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child %d was killed by signal %d\n", 
                   pid, WTERMSIG(status));
        }
    }
    
    printf("All children completed\n");
    return EXIT_SUCCESS;
} 
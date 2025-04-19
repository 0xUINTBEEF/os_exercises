/**
 * Process Creation Demonstration
 * 
 * This program demonstrates the creation of child and grandchild processes
 * using the fork() system call. It shows how process IDs are inherited
 * and how the process hierarchy works in Unix-like systems.
 * 
 * Features:
 * - Process hierarchy creation
 * - PID inheritance demonstration
 * - Error handling
 * - Resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

// Constants
#define CHILD_SLEEP_TIME 2
#define GRANDCHILD_SLEEP_TIME 1

// Global variables for cleanup
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * @brief Main function demonstrating process creation
 * 
 * Creates a child process and a grandchild process using fork().
 * Demonstrates the process hierarchy and PID inheritance.
 * 
 * @return int Exit status (0 on success)
 */
int main(void)
{
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("Main process (PID: %d, PPID: %d) starting...\n", 
           getpid(), getppid());
    
    // Create child process
    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        // Child process
        printf("Child process (PID: %d, PPID: %d) created\n", 
               getpid(), getppid());
        
        // Create grandchild process
        pid_t grandchild_pid = fork();
        if (grandchild_pid < 0) {
            perror("fork failed in child");
            return EXIT_FAILURE;
        }

        if (grandchild_pid == 0) {
            // Grandchild process
            printf("Grandchild process (PID: %d, PPID: %d) created\n", 
                   getpid(), getppid());
            sleep(GRANDCHILD_SLEEP_TIME);
            printf("Grandchild process terminating\n");
            return EXIT_SUCCESS;
        } else {
            // Child process continues
            printf("Child process created grandchild with PID: %d\n", 
                   grandchild_pid);
            
            // Wait for grandchild
            int status;
            pid_t waited_pid = waitpid(grandchild_pid, &status, 0);
            
            if (waited_pid == -1) {
                perror("waitpid failed in child");
                return EXIT_FAILURE;
            }
            
            if (WIFEXITED(status)) {
                printf("Grandchild process %d exited with status %d\n", 
                       waited_pid, WEXITSTATUS(status));
            }
            
            sleep(CHILD_SLEEP_TIME);
            printf("Child process terminating\n");
            return EXIT_SUCCESS;
        }
    } else {
        // Parent process
        printf("Main process created child with PID: %d\n", child_pid);
        
        // Wait for child
        int status;
        pid_t waited_pid = waitpid(child_pid, &status, 0);
        
        if (waited_pid == -1) {
            perror("waitpid failed in parent");
            return EXIT_FAILURE;
        }
        
        if (WIFEXITED(status)) {
            printf("Child process %d exited with status %d\n", 
                   waited_pid, WEXITSTATUS(status));
        }
    }

    printf("Main process terminating\n");
    return EXIT_SUCCESS;
}
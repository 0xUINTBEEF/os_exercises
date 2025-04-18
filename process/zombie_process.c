/**
 * @file zombie_process.c
 * @brief Demonstration of zombie process management
 * 
 * This program demonstrates how zombie processes are created and
 * how to prevent them using proper process management techniques.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define NUM_CHILDREN 3
#define CHILD_LIFETIME 2

/**
 * @brief Signal handler for SIGCHLD
 * 
 * @param signum Signal number
 */
void sigchld_handler(int signum) {
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

/**
 * @brief Child process function
 * 
 * @return int Exit status
 */
int child_process(void) {
    printf("Child %d started\n", getpid());
    
    // Simulate some work
    sleep(CHILD_LIFETIME);
    
    printf("Child %d completed\n", getpid());
    return 0;
}

/**
 * @brief Main function
 */
int main(void) {
    pid_t children[NUM_CHILDREN];
    
    // Set up SIGCHLD handler
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }
    
    // Create child processes
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("fork");
            return 1;
        } else if (pid == 0) {
            // Child process
            return child_process();
        } else {
            // Parent process
            children[i] = pid;
            printf("Created child process %d\n", pid);
        }
    }
    
    // Parent process continues
    printf("Parent process %d waiting for children...\n", getpid());
    
    // Wait for all children to complete
    for (int i = 0; i < NUM_CHILDREN; i++) {
        int status;
        pid_t pid = waitpid(children[i], &status, 0);
        
        if (pid == -1) {
            perror("waitpid");
        } else if (WIFEXITED(status)) {
            printf("Child %d exited normally with status %d\n", 
                   pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child %d was killed by signal %d\n", 
                   pid, WTERMSIG(status));
        }
    }
    
    printf("All children completed\n");
    return 0;
} 
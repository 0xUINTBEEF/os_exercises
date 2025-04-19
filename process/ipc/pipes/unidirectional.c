/**
 * Unidirectional Pipe Example
 * 
 * This program demonstrates one-way communication between parent and child processes
 * using an unnamed pipe. The parent writes to the pipe and the child reads from it.
 * 
 * Features:
 * - Unidirectional communication
 * - Error handling
 * - Graceful shutdown
 * - Safe buffer management
 * - Resource cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

// Constants
#define BUFFER_SIZE 128
#define NUM_MESSAGES 3

// Global variables for cleanup
static int pipefd[2] = {-1, -1};
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Cleanup function
static void cleanup(void) {
    if (pipefd[0] != -1) close(pipefd[0]);
    if (pipefd[1] != -1) close(pipefd[1]);
}

int main(void) {
    pid_t child;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int status;

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Create pipe
    if (pipe(pipefd) < 0) {
        perror("pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Fork process
    child = fork();
    if (child < 0) {
        perror("fork failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (child == 0) {  // Child process (reader)
        // Close write end of pipe
        close(pipefd[1]);

        printf("Child process started\n");

        // Read messages from pipe
        while (running && (bytes_read = read(pipefd[0], buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("Child received: %s", buffer);
            memset(buffer, 0, BUFFER_SIZE);
        }

        if (bytes_read < 0 && errno != EINTR) {
            perror("child: read error");
            cleanup();
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        printf("Child process finished\n");
        exit(EXIT_SUCCESS);
    } else {  // Parent process (writer)
        // Close read end of pipe
        close(pipefd[0]);

        printf("Parent process started\n");

        // Write messages to pipe
        const char *messages[] = {
            "Hello, World!\n",
            "Morning!\n",
            "Goodbye, World!\n"
        };

        for (int i = 0; i < NUM_MESSAGES && running; i++) {
            if (write(pipefd[1], messages[i], strlen(messages[i])) < 0) {
                perror("parent: write error");
                cleanup();
                exit(EXIT_FAILURE);
            }
        }

        close(pipefd[1]);

        // Wait for child to finish
        printf("Waiting for child process...\n");
        wait(&status);

        if (WIFEXITED(status)) {
            printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        } else {
            printf("Child process terminated abnormally\n");
        }

        printf("Parent process finished\n");
    }

    cleanup();
    return EXIT_SUCCESS;
}
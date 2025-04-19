/**
 * Bidirectional Pipe Communication Example
 * 
 * This program demonstrates bidirectional communication between parent and child processes
 * using two pipes. One pipe for parent-to-child communication and another for child-to-parent.
 * 
 * Features:
 * - Bidirectional communication
 * - Error handling
 * - Graceful shutdown
 * - Safe buffer management
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
#define PARENT_TO_CHILD_MESSAGES 3
#define CHILD_TO_PARENT_MESSAGES 2

// Global variables for cleanup
static int fd1[2] = {-1, -1};
static int fd2[2] = {-1, -1};
static volatile sig_atomic_t running = 1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Cleanup function
static void cleanup(void) {
    if (fd1[0] != -1) close(fd1[0]);
    if (fd1[1] != -1) close(fd1[1]);
    if (fd2[0] != -1) close(fd2[0]);
    if (fd2[1] != -1) close(fd2[1]);
}

int main(void) {
    pid_t child;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int status;

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Create pipes
    if (pipe(fd1) < 0 || pipe(fd2) < 0) {
        perror("pipe creation failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Fork process
    child = fork();
    if (child < 0) {
        perror("fork failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (child == 0) {  // Child process
        // Close unused ends
        close(fd1[1]);
        close(fd2[0]);

        printf("Child process started\n");

        // Read messages from parent
        while (running && (bytes_read = read(fd1[0], buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("Child received: %s", buffer);
        }

        if (bytes_read < 0 && errno != EINTR) {
            perror("child read error");
            cleanup();
            exit(EXIT_FAILURE);
        }

        close(fd1[0]);

        // Write messages to parent
        const char *messages[] = {
            "Hello, Parent!\n",
            "Goodbye, Parent!\n"
        };

        for (int i = 0; i < CHILD_TO_PARENT_MESSAGES && running; i++) {
            if (write(fd2[1], messages[i], strlen(messages[i])) < 0) {
                perror("child write error");
                cleanup();
                exit(EXIT_FAILURE);
            }
        }

        close(fd2[1]);
        printf("Child process finished\n");
        exit(EXIT_SUCCESS);
    } else {  // Parent process
        // Close unused ends
        close(fd1[0]);
        close(fd2[1]);

        printf("Parent process started\n");

        // Write messages to child
        const char *messages[] = {
            "Hello, World!\n",
            "Morning!\n",
            "Goodbye, World!\n"
        };

        for (int i = 0; i < PARENT_TO_CHILD_MESSAGES && running; i++) {
            if (write(fd1[1], messages[i], strlen(messages[i])) < 0) {
                perror("parent write error");
                cleanup();
                exit(EXIT_FAILURE);
            }
        }

        close(fd1[1]);

        // Wait for child to finish writing
        printf("Waiting for child process...\n");
        wait(&status);

        // Read messages from child
        while (running && (bytes_read = read(fd2[0], buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("Parent received: %s", buffer);
        }

        if (bytes_read < 0 && errno != EINTR) {
            perror("parent read error");
            cleanup();
            exit(EXIT_FAILURE);
        }

        close(fd2[0]);

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
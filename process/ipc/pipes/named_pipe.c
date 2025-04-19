/**
 * Named Pipe (FIFO) Example
 * 
 * This program demonstrates communication between parent and child processes
 * using a named pipe (FIFO). The parent writes to the pipe and the child reads from it.
 * 
 * Features:
 * - Named pipe creation and management
 * - Error handling
 * - Graceful shutdown
 * - Safe buffer management
 * - Resource cleanup
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

// Constants
#define PIPE_NAME "named_pipe"
#define BUFFER_SIZE 128
#define NUM_MESSAGES 3
#define PIPE_PERMISSIONS 0666

// Global variables for cleanup
static volatile sig_atomic_t running = 1;
static int pipe_fd = -1;

// Signal handler for graceful shutdown
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// Cleanup function
static void cleanup(void) {
    if (pipe_fd != -1) {
        close(pipe_fd);
    }
    unlink(PIPE_NAME);
}

int main(void) {
    pid_t reader;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int status;

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Create named pipe
    if (mkfifo(PIPE_NAME, PIPE_PERMISSIONS) < 0) {
        if (errno != EEXIST) {
            perror("mkfifo failed");
            exit(EXIT_FAILURE);
        }
    }

    // Fork process
    reader = fork();
    if (reader < 0) {
        perror("fork failed");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (reader == 0) {  // Child process (reader)
        printf("Child process started\n");

        // Open pipe for reading
        pipe_fd = open(PIPE_NAME, O_RDONLY);
        if (pipe_fd < 0) {
            perror("child: open failed");
            cleanup();
            exit(EXIT_FAILURE);
        }

        // Read messages from pipe
        while (running && (bytes_read = read(pipe_fd, buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("Child received: %s", buffer);
            memset(buffer, 0, BUFFER_SIZE);
        }

        if (bytes_read < 0 && errno != EINTR) {
            perror("child: read error");
            cleanup();
            exit(EXIT_FAILURE);
        }

        close(pipe_fd);
        printf("Child process finished\n");
        exit(EXIT_SUCCESS);
    } else {  // Parent process (writer)
        printf("Parent process started\n");

        // Open pipe for writing
        pipe_fd = open(PIPE_NAME, O_WRONLY);
        if (pipe_fd < 0) {
            perror("parent: open failed");
            cleanup();
            exit(EXIT_FAILURE);
        }

        // Write messages to pipe
        const char *messages[] = {
            "Hello, World!\n",
            "Morning!\n",
            "Goodbye, World!\n"
        };

        for (int i = 0; i < NUM_MESSAGES && running; i++) {
            if (write(pipe_fd, messages[i], strlen(messages[i])) < 0) {
                perror("parent: write error");
                cleanup();
                exit(EXIT_FAILURE);
            }
        }

        close(pipe_fd);

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
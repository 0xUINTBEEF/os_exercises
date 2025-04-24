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
#include <time.h>

// Constants
#define PIPE_NAME "named_pipe"
#define BUFFER_SIZE 128
#define NUM_MESSAGES 3
#define PIPE_PERMISSIONS 0666
#define TIMEOUT_SECONDS 5
#define LOG_FILE "named_pipe.log"

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

// Function prototypes
static void log_message(const char *message);
static int wait_for_pipe(int fd, int timeout);

// Function to log messages to a file
static void log_message(const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "%s: %s\n", ctime(&now), message);
        fclose(log_file);
    }
}

// Function to wait for pipe readiness with timeout
static int wait_for_pipe(int fd, int timeout) {
    fd_set set;
    struct timeval tv;

    FD_ZERO(&set);
    FD_SET(fd, &set);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return select(fd + 1, &set, NULL, NULL, &tv);
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
        log_message("Child process started");

        // Open pipe for reading
        pipe_fd = open(PIPE_NAME, O_RDONLY);
        if (pipe_fd < 0) {
            perror("child: open failed");
            log_message("Child: open failed");
            cleanup();
            exit(EXIT_FAILURE);
        }

        // Read messages from pipe
        while (running) {
            int ready = wait_for_pipe(pipe_fd, TIMEOUT_SECONDS);
            if (ready == 0) {
                printf("Child: Timeout waiting for data\n");
                log_message("Child: Timeout waiting for data");
                break;
            } else if (ready < 0) {
                perror("child: select error");
                log_message("Child: select error");
                break;
            }

            bytes_read = read(pipe_fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf("Child received: %s", buffer);
                log_message(buffer);
                memset(buffer, 0, BUFFER_SIZE);
            } else if (bytes_read < 0 && errno != EINTR) {
                perror("child: read error");
                log_message("Child: read error");
                break;
            }
        }

        close(pipe_fd);
        printf("Child process finished\n");
        log_message("Child process finished");
        exit(EXIT_SUCCESS);
    } else {  // Parent process (writer)
        printf("Parent process started\n");
        log_message("Parent process started");

        // Open pipe for writing
        pipe_fd = open(PIPE_NAME, O_WRONLY);
        if (pipe_fd < 0) {
            perror("parent: open failed");
            log_message("Parent: open failed");
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
                log_message("Parent: write error");
                cleanup();
                exit(EXIT_FAILURE);
            }
            log_message(messages[i]);
        }

        close(pipe_fd);

        // Wait for child to finish
        printf("Waiting for child process...\n");
        log_message("Waiting for child process...");
        wait(&status);

        if (WIFEXITED(status)) {
            printf("Child process exited with status: %d\n", WEXITSTATUS(status));
            log_message("Child process exited successfully");
        } else {
            printf("Child process terminated abnormally\n");
            log_message("Child process terminated abnormally");
        }

        printf("Parent process finished\n");
        log_message("Parent process finished");
    }

    cleanup();
    return EXIT_SUCCESS;
}
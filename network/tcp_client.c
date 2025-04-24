/**
 * TCP Client Implementation with Enhanced Features
 * 
 * This program demonstrates a robust TCP client implementation with:
 * - Connection management
 * - Error handling and logging
 * - Resource management
 * - User input handling
 * - Graceful shutdown
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

// Constants
#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080
#define DEFAULT_HOST "127.0.0.1"
#define TIMEOUT_SECONDS 5
#define LOG_FILE "tcp_client.log"

// Global variables
static volatile sig_atomic_t running = 1;
static int client_socket = -1;

/**
 * Signal handler for graceful shutdown
 * @param sig Signal number
 */
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * Function to log messages to a file
 * @param message Message to log
 */
static void log_message(const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "%s: %s\n", ctime(&now), message);
        fclose(log_file);
    }
}

/**
 * Initialize client socket
 * @param host Server hostname or IP address
 * @param port Server port number
 * @return 0 on success, -1 on error
 */
static int init_client(const char *host, int port) {
    struct sockaddr_in server_addr;
    struct timeval timeout;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket");
        return -1;
    }

    // Set socket timeout
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(client_socket);
        return -1;
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert hostname to IP address
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", host);
        close(client_socket);
        return -1;
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(client_socket);
        return -1;
    }

    return 0;
}

/**
 * Send data to server
 * @param buffer Data to send
 * @param length Length of data
 * @return Number of bytes sent, -1 on error
 */
static ssize_t send_data(const char *buffer, size_t length) {
    ssize_t total_sent = 0;
    ssize_t bytes_sent;

    while (total_sent < (ssize_t)length) {
        bytes_sent = send(client_socket, buffer + total_sent, length - total_sent, 0);
        if (bytes_sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("send");
            return -1;
        }
        total_sent += bytes_sent;
    }

    return total_sent;
}

/**
 * Receive data from server
 * @param buffer Buffer to store received data
 * @param max_length Maximum length of buffer
 * @return Number of bytes received, -1 on error
 */
static ssize_t receive_data(char *buffer, size_t max_length) {
    ssize_t bytes_received = recv(client_socket, buffer, max_length - 1, 0);
    if (bytes_received < 0) {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        perror("recv");
        return -1;
    }
    buffer[bytes_received] = '\0';
    return bytes_received;
}

/**
 * Main function implementing TCP client
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    ssize_t bytes_received;

    // Parse command line arguments
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize client
    if (init_client(host, port) < 0) {
        return 1;
    }

    log_message("TCP client started");

    printf("Connected to server %s:%d\n", host, port);
    printf("Type 'quit' to exit\n");

    // Main client loop
    while (running) {
        printf("Enter message: ");
        fflush(stdout);

        // Read user input
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        // Remove newline character
        size_t length = strlen(buffer);
        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
        }

        // Check for quit command
        if (strcmp(buffer, "quit") == 0) {
            break;
        }

        // Send data to server
        if (send_data(buffer, strlen(buffer)) < 0) {
            break;
        }

        // Receive response from server
        bytes_received = receive_data(buffer, BUFFER_SIZE);
        if (bytes_received < 0) {
            break;
        } else if (bytes_received > 0) {
            printf("Server response: %s\n", buffer);
        }
    }

    // Cleanup
    printf("Disconnecting from server...\n");
    if (client_socket != -1) {
        close(client_socket);
    }

    log_message("TCP client finished");

    return 0;
}
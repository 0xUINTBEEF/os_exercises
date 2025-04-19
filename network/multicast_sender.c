/**
 * Multicast Sender Implementation with Enhanced Features
 * 
 * This program demonstrates a robust multicast sender implementation with:
 * - Multicast group communication
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

// Constants
#define BUFFER_SIZE 1024
#define MULTICAST_GROUP "239.0.0.1"
#define MULTICAST_PORT 8888
#define TTL 32

// Global variables
static volatile sig_atomic_t running = 1;
static int sender_socket = -1;

/**
 * Signal handler for graceful shutdown
 * @param sig Signal number
 */
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * Initialize multicast sender
 * @return 0 on success, -1 on error
 */
static int init_sender(void) {
    struct sockaddr_in multicast_addr;
    int ttl = TTL;

    // Create socket
    sender_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender_socket < 0) {
        perror("socket");
        return -1;
    }

    // Set TTL
    if (setsockopt(sender_socket, IPPROTO_IP, IP_MULTICAST_TTL,
                  &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt TTL");
        close(sender_socket);
        return -1;
    }

    // Initialize multicast address structure
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_addr.sin_port = htons(MULTICAST_PORT);

    // Bind socket
    if (bind(sender_socket, (struct sockaddr *)&multicast_addr,
            sizeof(multicast_addr)) < 0) {
        perror("bind");
        close(sender_socket);
        return -1;
    }

    return 0;
}

/**
 * Send multicast message
 * @param buffer Message to send
 * @param length Length of message
 * @return Number of bytes sent, -1 on error
 */
static ssize_t send_multicast(const char *buffer, size_t length) {
    struct sockaddr_in multicast_addr;

    // Initialize multicast address structure
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_addr.sin_port = htons(MULTICAST_PORT);

    // Send message
    ssize_t bytes_sent = sendto(sender_socket, buffer, length, 0,
                               (struct sockaddr *)&multicast_addr,
                               sizeof(multicast_addr));
    if (bytes_sent < 0) {
        perror("sendto");
    }
    return bytes_sent;
}

/**
 * Main function implementing multicast sender
 * @return 0 on success, 1 on error
 */
int main(void) {
    char buffer[BUFFER_SIZE];

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize sender
    if (init_sender() < 0) {
        return 1;
    }

    printf("Multicast Sender started\n");
    printf("Group: %s, Port: %d\n", MULTICAST_GROUP, MULTICAST_PORT);
    printf("Type 'quit' to exit\n");

    // Main sender loop
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

        // Send multicast message
        if (send_multicast(buffer, strlen(buffer)) < 0) {
            break;
        }

        printf("Message sent to multicast group\n");
    }

    // Cleanup
    printf("Shutting down multicast sender...\n");
    if (sender_socket != -1) {
        close(sender_socket);
    }

    return 0;
} 
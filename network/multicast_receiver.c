/**
 * Multicast Receiver Implementation with Enhanced Features
 * 
 * This program demonstrates a robust multicast receiver implementation with:
 * - Multicast group communication
 * - Error handling and logging
 * - Resource management
 * - Message processing
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

// Global variables
static volatile sig_atomic_t running = 1;
static int receiver_socket = -1;

/**
 * Signal handler for graceful shutdown
 * @param sig Signal number
 */
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * Initialize multicast receiver
 * @return 0 on success, -1 on error
 */
static int init_receiver(void) {
    struct sockaddr_in local_addr;
    struct ip_mreq mreq;
    int reuse = 1;

    // Create socket
    receiver_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (receiver_socket < 0) {
        perror("socket");
        return -1;
    }

    // Allow multiple sockets to use the same port
    if (setsockopt(receiver_socket, SOL_SOCKET, SO_REUSEADDR,
                  &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(receiver_socket);
        return -1;
    }

    // Initialize local address structure
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(MULTICAST_PORT);

    // Bind socket
    if (bind(receiver_socket, (struct sockaddr *)&local_addr,
            sizeof(local_addr)) < 0) {
        perror("bind");
        close(receiver_socket);
        return -1;
    }

    // Join multicast group
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(receiver_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                  &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt IP_ADD_MEMBERSHIP");
        close(receiver_socket);
        return -1;
    }

    return 0;
}

/**
 * Receive multicast message
 * @param buffer Buffer to store received message
 * @param max_length Maximum length of buffer
 * @return Number of bytes received, -1 on error
 */
static ssize_t receive_multicast(char *buffer, size_t max_length) {
    ssize_t bytes_received = recvfrom(receiver_socket, buffer, max_length - 1, 0,
                                     NULL, NULL);
    if (bytes_received < 0) {
        if (errno == EINTR) {
            return 0;
        }
        perror("recvfrom");
        return -1;
    }
    buffer[bytes_received] = '\0';
    return bytes_received;
}

/**
 * Main function implementing multicast receiver
 * @return 0 on success, 1 on error
 */
int main(void) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize receiver
    if (init_receiver() < 0) {
        return 1;
    }

    printf("Multicast Receiver started\n");
    printf("Group: %s, Port: %d\n", MULTICAST_GROUP, MULTICAST_PORT);
    printf("Press Ctrl+C to exit\n");

    // Main receiver loop
    while (running) {
        bytes_received = receive_multicast(buffer, BUFFER_SIZE);
        if (bytes_received < 0) {
            break;
        } else if (bytes_received > 0) {
            printf("Received message: %s\n", buffer);
        }
    }

    // Cleanup
    printf("Shutting down multicast receiver...\n");
    if (receiver_socket != -1) {
        close(receiver_socket);
    }

    return 0;
} 
/**
 * UDP Server Implementation with Enhanced Features
 * 
 * This program demonstrates a robust UDP server implementation with:
 * - Connectionless communication
 * - Error handling and logging
 * - Resource management
 * - Client tracking
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
#define MAX_CLIENTS 100
#define TIMEOUT_SECONDS 30

// Structure to hold client information
typedef struct {
    struct sockaddr_in address;
    time_t last_activity;
    int active;
} client_info_t;

// Global variables
static volatile sig_atomic_t running = 1;
static int server_socket = -1;
static client_info_t clients[MAX_CLIENTS];

/**
 * Signal handler for graceful shutdown
 * @param sig Signal number
 */
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * Initialize server socket
 * @param port Server port number
 * @return 0 on success, -1 on error
 */
static int init_server(int port) {
    struct sockaddr_in server_addr;
    int opt = 1;

    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return -1;
    }

    // Set socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket);
        return -1;
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return -1;
    }

    return 0;
}

/**
 * Find or create client entry
 * @param client_addr Client address
 * @return Index of client entry, -1 if no space available
 */
static int find_client(const struct sockaddr_in *client_addr) {
    time_t current_time = time(NULL);
    int empty_slot = -1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            // Check for timeout
            if (current_time - clients[i].last_activity > TIMEOUT_SECONDS) {
                clients[i].active = 0;
                empty_slot = i;
                continue;
            }

            // Check if this is the client
            if (memcmp(&clients[i].address, client_addr, sizeof(*client_addr)) == 0) {
                clients[i].last_activity = current_time;
                return i;
            }
        } else if (empty_slot == -1) {
            empty_slot = i;
        }
    }

    // If we found an empty slot, use it
    if (empty_slot != -1) {
        clients[empty_slot].address = *client_addr;
        clients[empty_slot].last_activity = current_time;
        clients[empty_slot].active = 1;
        return empty_slot;
    }

    return -1;
}

/**
 * Send data to client
 * @param client_addr Client address
 * @param buffer Data to send
 * @param length Length of data
 * @return Number of bytes sent, -1 on error
 */
static ssize_t send_to_client(const struct sockaddr_in *client_addr,
                            const char *buffer,
                            size_t length) {
    ssize_t bytes_sent = sendto(server_socket, buffer, length, 0,
                               (struct sockaddr *)client_addr,
                               sizeof(*client_addr));
    if (bytes_sent < 0) {
        perror("sendto");
    }
    return bytes_sent;
}

/**
 * Main function implementing UDP server
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len;
    ssize_t bytes_received;
    int port = DEFAULT_PORT;
    int client_index;

    // Parse command line arguments
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Initialize server
    if (init_server(port) < 0) {
        return 1;
    }

    printf("UDP Server listening on port %d...\n", port);

    // Initialize clients array
    memset(clients, 0, sizeof(clients));

    // Main server loop
    while (running) {
        client_len = sizeof(client_addr);
        bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE - 1, 0,
                                (struct sockaddr *)&client_addr, &client_len);
        
        if (bytes_received < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("recvfrom");
            continue;
        }

        // Null-terminate received data
        buffer[bytes_received] = '\0';

        // Find or create client entry
        client_index = find_client(&client_addr);
        if (client_index == -1) {
            printf("Maximum clients reached, dropping packet\n");
            continue;
        }

        // Print received message
        printf("Received from %s:%d: %s\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               buffer);

        // Echo back to client
        if (send_to_client(&client_addr, buffer, bytes_received) < 0) {
            printf("Failed to send response to client\n");
        }
    }

    // Cleanup
    printf("Shutting down server...\n");
    if (server_socket != -1) {
        close(server_socket);
    }

    return 0;
} 
} 
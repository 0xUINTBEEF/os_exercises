/**
 * TCP Server Implementation with Enhanced Features
 * 
 * This program demonstrates a robust TCP server implementation with:
 * - Multiple client handling
 * - Graceful shutdown
 * - Error handling and logging
 * - Resource management
 * - Client connection tracking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define LOG_FILE "tcp_server.log"

// Constants
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 8080
#define BACKLOG 5
#define TIMEOUT_SECONDS 30

// Structure to hold client information
typedef struct {
    int socket;
    struct sockaddr_in address;
    time_t last_activity;
    pthread_t thread_id;
    int active;
} client_info_t;

// Global variables
static client_info_t clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static int server_socket;
static volatile sig_atomic_t running = 1;

/**
 * Signal handler for graceful shutdown
 * @param sig Signal number
 */
static void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

/**
 * Initialize client information
 */
static void init_clients(void) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;
        clients[i].active = 0;
    }
    pthread_mutex_unlock(&clients_mutex);
}

/**
 * Find available client slot
 * @return Index of available slot, -1 if none available
 */
static int find_available_slot(void) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            pthread_mutex_unlock(&clients_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return -1;
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
 * Handle client connection
 * @param arg Client information structure
 * @return NULL
 */
static void *handle_client(void *arg) {
    client_info_t *client = (client_info_t *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    time_t current_time;

    printf("Client connected: %s:%d\n",
           inet_ntoa(client->address.sin_addr),
           ntohs(client->address.sin_port));

    while (running) {
        // Check for timeout
        current_time = time(NULL);
        if (current_time - client->last_activity > TIMEOUT_SECONDS) {
            printf("Client timeout: %s:%d\n",
                   inet_ntoa(client->address.sin_addr),
                   ntohs(client->address.sin_port));
            break;
        }

        // Receive data
        bytes_received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected: %s:%d\n",
                       inet_ntoa(client->address.sin_addr),
                       ntohs(client->address.sin_port));
            } else {
                perror("recv");
            }
            break;
        }

        // Update last activity time
        client->last_activity = current_time;

        // Null-terminate received data
        buffer[bytes_received] = '\0';

        // Echo back to client
        if (send(client->socket, buffer, bytes_received, 0) < 0) {
            perror("send");
            break;
        }
    }

    // Cleanup
    close(client->socket);
    client->active = 0;

    return NULL;
}

/**
 * Main function implementing TCP server
 * @return 0 on success, 1 on error
 */
int main(void) {
    struct sockaddr_in server_addr;
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int slot;
    int opt = 1;

    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return 1;
    }

    // Set socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket);
        return 1;
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return 1;
    }

    // Listen for connections
    if (listen(server_socket, BACKLOG) < 0) {
        perror("listen");
        close(server_socket);
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);
    log_message("TCP server started");

    // Initialize clients array
    init_clients();

    // Main server loop
    while (running) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            continue;
        }

        // Find available slot
        slot = find_available_slot();
        if (slot == -1) {
            printf("No available slots for new client\n");
            close(client_socket);
            continue;
        }

        // Initialize client information
        pthread_mutex_lock(&clients_mutex);
        clients[slot].socket = client_socket;
        clients[slot].address = client_addr;
        clients[slot].last_activity = time(NULL);
        clients[slot].active = 1;
        pthread_mutex_unlock(&clients_mutex);

        // Create thread for client
        if (pthread_create(&clients[slot].thread_id, NULL, handle_client, &clients[slot]) != 0) {
            perror("pthread_create");
            close(client_socket);
            clients[slot].active = 0;
        }
    }

    // Cleanup
    printf("Shutting down server...\n");
    log_message("TCP server finished");
    
    // Close all client connections
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            close(clients[i].socket);
            pthread_join(clients[i].thread_id, NULL);
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    // Close server socket
    close(server_socket);

    return 0;
}
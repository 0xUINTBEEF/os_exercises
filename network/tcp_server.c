/**
 * TCP Server Implementation
 * 
 * This program demonstrates a basic TCP server that:
 * - Accepts multiple client connections
 * - Handles client requests concurrently
 * - Implements a simple echo service
 * - Provides connection statistics
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Structure to hold client information
typedef struct {
    int socket;
    struct sockaddr_in address;
    int id;
} client_t;

// Global variables
client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int num_clients = 0;

// Function to handle client communication
void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    printf("Client %d connected from %s:%d\n", 
           client->id, 
           inet_ntoa(client->address.sin_addr),
           ntohs(client->address.sin_port));

    // Client communication loop
    while ((bytes_received = recv(client->socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Received from client %d: %s\n", client->id, buffer);

        // Echo back to client
        if (send(client->socket, buffer, bytes_received, 0) < 0) {
            perror("Send failed");
            break;
        }
    }

    // Clean up client connection
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].id == client->id) {
            clients[i] = clients[--num_clients];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("Client %d disconnected\n", client->id);
    close(client->socket);
    free(client);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Configure address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("TCP Server listening on port %d\n", PORT);

    // Main server loop
    while (1) {
        client_t *client = malloc(sizeof(client_t));
        client->socket = accept(server_fd, (struct sockaddr *)&client->address, (socklen_t*)&addrlen);
        
        if (client->socket < 0) {
            perror("Accept failed");
            free(client);
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (num_clients < MAX_CLIENTS) {
            client->id = num_clients;
            clients[num_clients++] = *client;
            pthread_mutex_unlock(&clients_mutex);

            // Create thread for client
            pthread_t thread;
            if (pthread_create(&thread, NULL, handle_client, client) != 0) {
                perror("Thread creation failed");
                close(client->socket);
                free(client);
            }
        } else {
            pthread_mutex_unlock(&clients_mutex);
            printf("Maximum clients reached. Connection rejected.\n");
            close(client->socket);
            free(client);
        }
    }

    return 0;
} 
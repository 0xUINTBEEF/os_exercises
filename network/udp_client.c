/**
 * UDP Client Implementation
 * 
 * This program demonstrates a basic UDP client that:
 * - Sends datagrams to a UDP server
 * - Receives responses from the server
 * - Handles communication errors
 * - Supports interactive message input
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("UDP Client connected to server at %s:%d\n", SERVER_IP, PORT);

    // Communication loop
    while (1) {
        printf("Enter message (or 'quit' to exit): ");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline

        if (strcmp(message, "quit") == 0) {
            break;
        }

        // Send message to server
        if (sendto(sockfd, message, strlen(message), 0,
                  (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Send failed");
            continue;
        }

        // Receive response from server
        socklen_t len = sizeof(servaddr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                        (struct sockaddr *)&servaddr, &len);
        
        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
    }

    close(sockfd);
    return 0;
} 
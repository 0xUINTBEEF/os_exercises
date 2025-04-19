/**
 * UDP Server Implementation
 * 
 * This program demonstrates a basic UDP server that:
 * - Listens for UDP datagrams
 * - Processes incoming messages
 * - Sends responses back to clients
 * - Handles multiple clients without connection state
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUFFER_SIZE];
    socklen_t len;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("UDP Server listening on port %d\n", PORT);

    // Main server loop
    while (1) {
        len = sizeof(cliaddr);
        
        // Receive message from client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, 
                        (struct sockaddr *)&cliaddr, &len);
        
        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        buffer[n] = '\0';
        printf("Received from %s:%d: %s\n", 
               inet_ntoa(cliaddr.sin_addr),
               ntohs(cliaddr.sin_port),
               buffer);

        // Send response back to client
        if (sendto(sockfd, buffer, n, 0,
                  (const struct sockaddr *)&cliaddr, len) < 0) {
            perror("Send failed");
        }
    }

    close(sockfd);
    return 0;
} 
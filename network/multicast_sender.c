/**
 * Multicast Sender Implementation
 * 
 * This program demonstrates a multicast sender that:
 * - Joins a multicast group
 * - Broadcasts messages to all group members
 * - Handles multicast-specific socket options
 * - Supports interactive message input
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MULTICAST_GROUP "239.255.1.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in multicast_addr;
    char message[BUFFER_SIZE];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure multicast address
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_addr.sin_port = htons(PORT);

    // Set TTL (Time To Live) for multicast packets
    unsigned char ttl = 1; // Limit to local network
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("Setting TTL failed");
        exit(EXIT_FAILURE);
    }

    printf("Multicast Sender ready to send to group %s:%d\n", MULTICAST_GROUP, PORT);

    // Communication loop
    while (1) {
        printf("Enter message (or 'quit' to exit): ");
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline

        if (strcmp(message, "quit") == 0) {
            break;
        }

        // Send message to multicast group
        if (sendto(sockfd, message, strlen(message), 0,
                  (struct sockaddr *)&multicast_addr, sizeof(multicast_addr)) < 0) {
            perror("Send failed");
            continue;
        }

        printf("Message sent to multicast group\n");
    }

    close(sockfd);
    return 0;
} 
/**
 * Multicast Receiver Implementation
 * 
 * This program demonstrates a multicast receiver that:
 * - Joins a multicast group
 * - Receives messages from the group
 * - Handles multicast-specific socket options
 * - Displays received messages
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
    struct sockaddr_in local_addr, multicast_addr;
    struct ip_mreq mreq;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure local address
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Join multicast group
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Joining multicast group failed");
        exit(EXIT_FAILURE);
    }

    printf("Multicast Receiver listening on group %s:%d\n", MULTICAST_GROUP, PORT);

    // Main receive loop
    while (1) {
        socklen_t addrlen = sizeof(multicast_addr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                        (struct sockaddr *)&multicast_addr, &addrlen);
        
        if (n < 0) {
            perror("Receive failed");
            continue;
        }

        buffer[n] = '\0';
        printf("Received from %s:%d: %s\n",
               inet_ntoa(multicast_addr.sin_addr),
               ntohs(multicast_addr.sin_port),
               buffer);
    }

    // Leave multicast group
    setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
    close(sockfd);
    return 0;
} 
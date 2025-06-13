#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 53
#define BUFFER_SIZE 512

int main() {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    unsigned char buffer[BUFFER_SIZE];
    socklen_t len = sizeof(client_addr);

    // Create UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind to port 53
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed (need root privileges for port 53)");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("DNS server running on port %d...\n", PORT);

    while (1) {
        int n = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &len);
        if (n < 0) {
            perror("Failed to receive data");
            continue;
        }

        // Basic response: just echoing query with a dummy answer
        buffer[2] |= 0x80;  // set QR flag (response)
        buffer[3] |= 0x80;  // recursion available

        // Set answer count to 1
        buffer[6] = 0x00;
        buffer[7] = 0x01;

        int qlen = n; // length of qcduestion section assumed to be entire packet
        int offset = qlen;

        // Append answer (point to name with 0xC00C)
        buffer[offset++] = 0xC0;
        buffer[offset++] = 0x0C;
        buffer[offset++] = 0x00; // TYPE A
        buffer[offset++] = 0x01;
        buffer[offset++] = 0x00; // CLASS IN
        buffer[offset++] = 0x01;
        buffer[offset++] = 0x00; buffer[offset++] = 0x00; buffer[offset++] = 0x00; buffer[offset++] = 0x3C; // TTL 60
        buffer[offset++] = 0x00; buffer[offset++] = 0x04; // RDLENGTH 4 bytes
        buffer[offset++] = 1; buffer[offset++] = 2; buffer[offset++] = 3; buffer[offset++] = 4; // IP = 1.2.3.4

        sendto(sock, buffer, offset, 0, (struct sockaddr *)&client_addr, len);
    }

    close(sock);
    return 0;
}

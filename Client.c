#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>
#pragma comment(lib, "Ws2_32.lib")

static void init_winsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        exit(EXIT_FAILURE);
    }
}

static void cleanup_winsock() {
    WSACleanup();
}

#define PORT 8080

int main() {
    init_winsock();

    int client_fd;
    struct sockaddr_in server_address;
    char buffer[1024] = {0};
    char message[1024] = {0};
    char exitmsg[] = "Exit Server";
    

    // Create the socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Connection failed: %d\n", WSAGetLastError());
        closesocket(client_fd);
        cleanup_winsock();
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server.\n");

    printf("Enter message : ");
    fgets(message, sizeof(message), stdin);  //change const character to user input text (using fgets so that it counts in white spaces)

    int len = strlen(message);         // removing new line character from fgets function
    if (message[len - 1] == '\n')
    {
        message[len - 1] = '\0';
    }

    
    // Send data to the server
    send(client_fd, message, strlen(message), 0);
    printf("Sent to server: %s\n", message); 

    // Receive response from the server
    recv(client_fd, buffer, sizeof(buffer), 0);
    printf("Received from server: %s\n", buffer);     
    
    // Close the socket
    closesocket(client_fd);
    cleanup_winsock();

    return 0;
}
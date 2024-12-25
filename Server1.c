#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>  
#include <ws2tcpip.h>   
#include <ctype.h>

#pragma comment(lib, "Ws2_32.lib")  

#define close(fd) closesocket(fd) 

// Automatic Winsock initialization and cleanup
static void init_winsock() __attribute__((constructor));
static void cleanup_winsock() __attribute__((destructor));

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

#define PORT 8080  // Port number for the server

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind socket to an IP/Port
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Any IP address
    address.sin_port = htons(PORT);        // Host to Network Short

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        
        perror("Listening failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", PORT);

    // 4. Accept a connection
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket == -1) {
        perror("Accepting connection failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 5. Receive data from the client
    recv(new_socket, buffer, sizeof(buffer), 0);
    printf("Received from client: %s\n", buffer);

    // 6. Modify the data
    
    int len = strlen(buffer);            
    for (size_t i = 0; i < len ; i++)   // loop to skip through digits and change capitalize upper case
    {
        if(isalpha(buffer[i]))
        {
            buffer[i] = toupper(buffer[i]);
        }
    }

    char temp;
    int start = 0;
    int end = len - 1;
    while (start < end)   // loop to reverse index Reference : Reverse String in C - geeksforgeeks (https://www.geeksforgeeks.org/reverse-string-in-c/)
    {
        temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }
       

    // 6. Send response to the client
    
    send(new_socket, buffer, strlen(buffer), 0);
    printf("Sent to client: %s\n", buffer);

    // 7. Close the sockets
    close(new_socket);
    close(server_fd);

    return 0;
}
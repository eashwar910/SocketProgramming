#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>

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

#define PORT 8080

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
    if (listen(server_fd, 3) < 0) {
        perror("Listening failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", PORT);

    while (1) {  //while loop to keep listening until exit message is given

        // 4. Accept a new connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket == -1) {
            perror("Accepting connection failed");
            continue;
        }

        // 5. Obtain and display IP and port number
        char *client_ip = inet_ntoa(address.sin_addr);
        int client_port = ntohs(address.sin_port);
        printf("Connection established with IP: %s, Port: %d\n", client_ip, client_port); // Reference : Network Sockets in C/C++ (https://www.keypuncher.net/blog/network-sockets-in-c)

        // 6. Error Handling to check if the recv function is returning a negative or a zero value 
        int bytes_received = recv(new_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {                                          // Reference : (https://www.demo2s.com/c/c-while-recv-connfd-buffer-buffer-size-0-0.html)
            printf("Client disconnected or error occurred.\n");  //
            close(new_socket);
            continue;
        }

        buffer[bytes_received] = '\0'; 
        printf("Received message: %s\n", buffer);

        // 7. Compare to Exit Message and disconnect
        if (strcmp(buffer, "Exit Server") == 0) {
            printf("Termination request received. Closing connection.\n");
            close(new_socket);
            break;
        }

        // 8. Validate input message and check if alphanumeric
        int is_alphanumeric = 1;
        for (int i = 0; i < strlen(buffer); i++) {
            if (!isalnum(buffer[i]) && !isspace(buffer[i])) {
                is_alphanumeric = 0;
                break;
            }
        }

        if (!is_alphanumeric) {
            char *error_msg = "Error: Message must be alphanumeric.";
            send(new_socket, error_msg, strlen(error_msg), 0);
            printf("Sent error message to client.\n");
            close(new_socket); // Close the connection for the client to re-prompt
            continue;
        }

        // 9. Data Modification
        int len = strlen(buffer);
        for (int i = 0; i < len; i++) {
            if (isalpha(buffer[i])) {
                buffer[i] = toupper(buffer[i]);
            }
        }

        for (int i = 0, j = len - 1; i < j; i++, j--) {
            char temp = buffer[i];
            buffer[i] = buffer[j];
            buffer[j] = temp;
        }


        // 10. Send reposnse to Client        
        printf("Message length: %d\n", len);
        send(new_socket, buffer, strlen(buffer), 0);
        printf("Sent to Client : %s\n", buffer);

        
        memset(buffer, 0, sizeof(buffer)); // Reference : (https://www.demo2s.com/c/c-while-recv-connfd-buffer-buffer-size-0-0.html)


        // 11. Close the connection 
        close(new_socket);

    }

    //12. Close the server socket
    close(server_fd);
    return 0;
}


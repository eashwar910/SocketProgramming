#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

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
    int exit_val = 0;

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

    while (1) {
        // 4. Accept a new connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket == -1) {
            perror("Accepting connection failed");
            continue;
        }

        // 5. Obtain and display IP and port number
        char *client_ip = inet_ntoa(address.sin_addr);
        int client_port = ntohs(address.sin_port);
        printf("Connection established with IP: %s, Port: %d\n", client_ip, client_port);

        // 6. Error Handling to check if the recv function is returning a negative or a zero value 
        int bytes_received = recv(new_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            printf("Client disconnected or error occurred.\n");
            close(new_socket);
            continue;
        }

        printf("Received message: %s\n", buffer);

        if (strcmp(buffer, "Date") == 0)
        {
            // 7. Obtaining localtime of system // Reference : (https://www.demo2s.com/cpp/cpp-strftime-s-128-a-d-b-y-00-00-00-0800-tm.html)
            char response[1024];
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);

            strftime(response, sizeof(response), "%a %b %d %H:%M:%S %Y GMT +0800\r\n", tm_info); // Setting time to GMT +8

            printf("Sending date and time: %s", response);
            send(new_socket, response, strlen(response), 0);

        }

        // 8. Compare to Exit Message and disconnect
        else if (strcmp(buffer, "Exit Server") == 0)
        {
            printf("Termination request received. Closing connection.\n");
            exit_val = 1;
            close(new_socket);
            break;
        }

        else
        {
            // 9. Invalidate any other command or input
            char response[1024];
            snprintf(response, sizeof(response), "Error: Invalid command. Use 'Date' or 'Exit Server'.\r\n");
            send(new_socket, response, strlen(response), 0);
        }

    }

    //10. Close the server socket
    close(server_fd);
    return 0;
}

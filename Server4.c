#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>

#pragma comment(lib, "Ws2_32.lib")

#define close(fd) closesocket(fd)
#define PORT 8080

// struct of timezone shorthands their respective offsets from loal time
struct TimeZone {
    char *name;
    int offset;
};

struct TimeZone time_zones[] = {
    {"PST", -8},
    {"MST", -7},
    {"CST", -6},
    {"EST", -5},
    {"GMT", 0},
    {"CET", 1},
    {"MSK", 3},
    {"JST", 9},
    {"AEDT", 11}
};
int num_tz = sizeof(time_zones) / sizeof(time_zones[0]);

static void init_winsock() __attribute__((constructor));
static void cleanup_winsock() __attribute__((destructor));

// Automatic Winsock initialization and cleanup
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

    while (1) {
        // 4. Accept a new connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket == -1) {
            perror("Accepting connection failed");
            continue;
        }

        // 5. Display client IP and port
        char *client_ip = inet_ntoa(address.sin_addr);
        int client_port = ntohs(address.sin_port);
        printf("Connection established with IP: %s, Port: %d\n", client_ip, client_port);

        int bytes_received = recv(new_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("Client disconnected or error occurred.\n");
            close(new_socket);
            continue;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received message: %s\n", buffer);

        // 6. Handle "Date" Command
        if (strcmp(buffer, "Date") == 0)
        {
            // 7. Obtaining localtime of system
            char response[1024];
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);

            strftime(response, sizeof(response), "%a %b %d %H:%M:%S %Y GMT +0800\r\n", tm_info); // Setting time to GMT +8

            printf("Sending date and time: %s", response);
            send(new_socket, response, strlen(response), 0);  

        }    

        // 6. Handle "Time" command
        if (strncmp(buffer, "Time", 4) == 0) {
            char *timezone = strtok(buffer + 5, " \r\n");
            int offset = 8; // Default Malaysia Time (GMT+8)

            if (timezone != NULL) {
                int valid = 0;
                for (int i = 0; i < num_tz; i++) {
                    if (strcmp(timezone, time_zones[i].name) == 0) {
                        offset = time_zones[i].offset;
                        valid = 1;
                        break;
                    }
                }                      
                if (!valid) {
                    char error_msg[] = "Error: Invalid time-zone code.\r\n";
                    send(new_socket, error_msg, strlen(error_msg), 0);
                    printf("Sent error message: Invalid time-zone.\n");
                    close(new_socket);  
                    continue;
                }
            }

            //7. Inline time formatting
            char response[1024];
            time_t t = time(NULL);
            t = t + offset * 3600; // Adjust time by the offset in seconds
            struct tm *tm_info = gmtime(&t); // Use GMT and adjust manually
            strftime(response, sizeof(response), "%H:%M:%S %Y %Z\r\n", tm_info); // Reference : (https://www.mankier.com/3/strftime)

            // 8. Send the formatted time to the client
            send(new_socket, response, strlen(response), 0);
            printf("Sent time for timezone: %s\n", timezone ? timezone : "Malaysia");
        }
        // 9. Compare to Exit Message and disconnect
        else if (strcmp(buffer, "Exit Server") == 0) {
            printf("Termination request received. Closing connection.\n");
            close(new_socket);
            break;
        }
        // 10. Handle invalid input
        else {
            char error_msg[] = "Error: Invalid command. Use 'Time <timezone>' or 'Date' or 'Exit Server'.\r\n";
            send(new_socket, error_msg, strlen(error_msg), 0);
            printf("Sent error message: Invalid command.\n");
        }

        close(new_socket); // Close the connection
    }

    close(server_fd); // Close the server socket
    return 0;
}

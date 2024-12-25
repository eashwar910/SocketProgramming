#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

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

#define PORT 8080 // 

// Structure to pass client socket to the thread function
typedef struct {
    SOCKET cli_sock;
    int cli_ID;
} c_data;

// Mutex for synchronizing access to the client sockets
HANDLE mutex_client = NULL;
SOCKET client_sockets[2];
int exit_ctr = 0;

// Function to forward messages
void ForwardMessage(SOCKET sender_socket, const char* message);

// Function to handle client communication
DWORD WINAPI ClientThread(LPVOID lpParam);

int main() {
    SOCKET server_fd, new_socket;
    struct sockaddr_in server_address, client_address;
    int cliadr_size = sizeof(client_address);

    //1.  Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);        
    }

    // 2. Bind socket to an IP/Port
    server_address.sin_family = AF_INET;          // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY;  // Any IP address
    server_address.sin_port = htons(PORT);        // Host to Network Short

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // 3. Listen for incoming connections
    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // 4. Initialize the client socket array
    for (int i = 0; i < 2; i++) {
        client_sockets[i] = INVALID_SOCKET;
    }

    // 5. create mutex to synchronize the socket
    mutex_client = CreateMutex(NULL, FALSE, NULL);

    printf("Server is listening on port %d...\n", PORT);

    // 5. Accept client connections
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&client_address, &cliadr_size);
        if (new_socket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed. Error Code: %d\n", WSAGetLastError());
            continue;
        }

        printf("Client connected.\n");

        // Find an empty slot for the new client
        int client_ID = -1;
        WaitForSingleObject(mutex_client, INFINITE);
        for (int i = 0; i < 2; i++) {
            if (client_sockets[i] == INVALID_SOCKET) {
                client_sockets[i] = new_socket;
                client_ID = i;
                break;
            }
        }
        ReleaseMutex(mutex_client);

        // If there's space for the client, create a new thread
        // Reference : (https://dotnettutorials.net/lesson/multithreading-in-c/)
        if (client_ID != -1) {
            // Allocate memory for client data
            c_data *clientData = (c_data *)malloc(sizeof(c_data));
            clientData->cli_sock = new_socket;
            clientData->cli_ID = client_ID;

            // Create a new thread for the client
            HANDLE thread = CreateThread(NULL, 0, ClientThread, clientData, 0, NULL);
            if (thread == NULL) {
                fprintf(stderr, "Thread creation failed. Error Code: %d\n", GetLastError());
                closesocket(new_socket);
                free(clientData);
                continue;
            }

            // Close the thread handle (it will still run independently)
            CloseHandle(thread);
        } else {
            // If no space, close the connection
            const char *message = "Server is full. Try again later.";
            send(new_socket, message, strlen(message), 0);
            closesocket(new_socket);
        }
    }

    // Close the server socket
    closesocket(server_fd);
    return 0;
}

void ForwardMessage(SOCKET senderSocket, const char* message) {
    // Lock the mutex to safely access the clientSockets array
    WaitForSingleObject(mutex_client, INFINITE);

    // Forward the message to the other client
    for (int i = 0; i < 2 ; i++) {
        if (client_sockets[i] != INVALID_SOCKET && client_sockets[i] != senderSocket) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }

    // Release the mutex after modifying the clientSockets array
    ReleaseMutex(mutex_client);
}

// Function to handle client communication
// Reference : (https://github.com/uzigun/Multi_Thread_Command_Line_Base_Chat_Application/blob/main/server.c)
DWORD WINAPI ClientThread(LPVOID lpParam) {
    c_data *clientData = (c_data *)lpParam;
    SOCKET clientSocket = clientData->cli_sock;
    char buffer[1024];
    int bytesRead;
    int exit_val = 0;

    // Send a welcome message to the client
    const char *welcomeMessage = "Welcome to the private chat room, You're connected to the other side now";
    send(clientSocket, welcomeMessage, strlen(welcomeMessage), 0);

    // Handle client communication
    while (1) {
        // Receive data from client
        bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';  // Null-terminate the received data
            printf("Received from client %d: %s\n", clientData->cli_ID, buffer);
        
            // Check if the client wants to exit
            if (strcmp(buffer, "Exit Server") == 0)
            {
                exit_val = clientData->cli_ID + 1;
                printf("Client %d exited the chat room.\n", exit_val);

                // Increment the exit counter
                WaitForSingleObject(mutex_client, INFINITE);
                exit_ctr++;
                ReleaseMutex(mutex_client);

                // Close the client socket
                closesocket(clientSocket);

                // Remove the client from the clientSockets array
                WaitForSingleObject(mutex_client, INFINITE);
                client_sockets[clientData->cli_ID] = INVALID_SOCKET;
                ReleaseMutex(mutex_client);

                // Check if both clients have exited
                if (exit_ctr >= 2) {
                    printf("Both clients have exited. Server shutting down...\n");
                    exit(0); // Terminate the server
                }

                // End the thread for this client
                break;
            }

            // Forward the message to the other client
            ForwardMessage(clientSocket, buffer);
        } else if (bytesRead == 0) {
            // Client closed the connection
            printf("Client %d disconnected.\n", clientData->cli_ID);
            break;
        } else {
            // Error occurred
            printf("Error receiving data from client %d.\n", clientData->cli_ID);
            break;
        }
    }

    // Free the memory for client data
    free(clientData);

    return 0;
}

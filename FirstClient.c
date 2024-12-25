#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") 

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

// message handling function 
DWORD WINAPI receiveMessages(LPVOID clientSocket);

int main() {
    init_winsock();
    SOCKET sock;
    struct sockaddr_in server_address;
    char message[1024];

    // Create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Connection to server failed. Error Code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server. Type 'Exit Server' to leave.\n");

    // Create a thread to receive messages
    HANDLE recvThread = CreateThread(NULL, 0, receiveMessages, &sock, 0, NULL);
    if (recvThread == NULL) {
        fprintf(stderr, "Failed to create receive thread. Error Code: %d\n", GetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // message handling thread loop
    while (1)
    {
        printf(">> "); 
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        // Send message to the server
        if (send(sock, message, strlen(message), 0) < 0)
        {
            fprintf(stderr, "Sending data failed. Error Code: %d\n", WSAGetLastError());
            break;
        }

        // Exit condition
        if (strcmp(message, "Exit Server") == 0)
        {
            printf("You have left the chat room.\n");
            break;
        }
    }

    // Wait for the receive thread to finish
    WaitForSingleObject(recvThread, INFINITE);

    // Cleanup
    closesocket(sock);
    WSACleanup();

    return 0;
}


// Reference : (https://github.com/uzigun/Multi_Thread_Command_Line_Base_Chat_Application/blob/main/server.c)

DWORD WINAPI receiveMessages(LPVOID clientSocket)
{
    SOCKET sock = *(SOCKET *)clientSocket;
    char buffer[1024];
    int valread;

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        // Receive message from the server
        valread = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (valread > 0)
        {
            buffer[valread] = '\0'; // Null-terminate the received data

            // Clear the current line and print the received message
            printf("\r%s\n", buffer);
            printf(">> "); // Redisplay the input prompt
            fflush(stdout);
        }
        else if (valread == 0)
        {
            printf("\nServer has closed the connection.\n");
            break;
        }
        else
        {
            fprintf(stderr, "Receiving data failed. Error Code: %d\n", WSAGetLastError());
            break;
        }
    }

    return 0;
}

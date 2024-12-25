# Socket Programming Project

This project demonstrates a simple client-server architecture using socket programming. The project consists of five servers and several client programs.

## Overview

- **Servers 1 - 4**: Share the same client.
- **Server 5**: Has two clients: `FirstClient` and `SecondClient`, each with separate connections.

## Files Overview

### Server Files

- **Server1.c**: This server handles a single client connection, modifies received messages, and sends the modified data back to the client.
- **Server2.c**: This is a modified version of server1, but it keeps the conection socket open and can accept any numberof connections until the user input is the exit message. Otherwise, the data is modified and sent back as usual.
- **Server3.c**: Modified from Server2, but only accepts commands as inputs. It can only take 2 commands, "Date"  which displays the current date time and in your timezone and the exit message.
- **Server4.c**: Modified from Server3, also takes in an extra command "Time" which can followed by a timezone "Time <timezone>. The following timezones have been implemented ![image](https://github.com/user-attachments/assets/406bfe22-d514-4a8c-9bcb-9feef3e0915b)
- **Server5.c**: This server supports two clients at the same time. It utilizes threads to handle communication with two clients. Messages from one client are forwarded to the other. Communication happens in real time. 

### Client Files

- **Client.c**: The client that can connect to any of the first four servers. It allows the user to send a message and receive a response.
- **FirstClient.c**: This client connects to Server5. It uses a separate communication thread to receive messages concurrently while sending messages.
- **SecondClient.c**: Another client for Server5, similar to `FirstClient`, with the same functionality.

## Requirements

- **Operating System**: Windows (requires Winsock library).
- **Compiler**: C compiler that supports Winsock (e.g., GCC, MSVC).
- **Motherboard**: AMD or Intel

- Please note that these server and client programs will only work on the speceified motherboard, for MacOS and Linux, the modules used and winsock initialization follows a different pattern. Otherwise, the functionality is the same.

## Setup and Usage

### 1. Compilation

To compile the programs, ensure that you have the `ws2_32.lib` library linked.

For example, to compile on GCC:
```
gcc -o server1 Server1.c -lws2_32
gcc -o client Client.c -lws2_32
```

### 2. Running the Servers

Each server should be run on a different terminal or machine (for the case of Server 5, which can handle multiple clients). You can run the server programs like so:

```
./server1
./server2
./server3
./server4
./server5
```

### 3. Running the Clients

You can run any of the clients to connect to the servers:

```
./client
./firstclient
./secondclient
```

### 4. Communication

- The client sends messages to the server, which processes the message and sends it back to the client.
- In Server 5, messages are forwarded between two clients connected to the server.

### 5. Exit

To close the client-server communication, send the message `Exit Server` from the client.


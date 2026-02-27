#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5
#define QUIT ":q"

int main() {
    int clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }
    int max_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    fd_set read_fds;

    char buffer[BUFFER_SIZE] = {0};

    // Create the socket and check for errors
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Prepare the sockaddr struct
    memset(&server_addr, 0, sizeof(server_addr)); // Zero out the struct

    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(PORT); // Port number in Network Byte Order

    // Use INADDR_ANY to bind to all available local interfaces (0.0.0.0)
    // Use inet_addr("127.0.0.1") to bind only to the loopback interface
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the port and IP
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Socket successfully bound to port %d\n", PORT);

    // Start listening to the socket
    if(listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Started listening on port %d\n", PORT);

    
    while(1) {
        FD_ZERO(&read_fds); // Set the temperary clients 

        // Add the server fd to the set
        FD_SET(server_fd, &read_fds);
        max_fd = server_fd; // max_fd is the latest client that connected 
        
        // Add all the current clients to the set
        for(int i = 0; i < MAX_CLIENTS; i++) {
            if(clients[i] != -1) {
                FD_SET(clients[i], &read_fds);
                if (clients[i] > max_fd) max_fd = clients[i];
            }
        }

        // ready is the amount of descriptors that are ready
        // select changes read_fds so it only contains the ready descriptors
        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if(ready < 0) {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Check if there is a new connection
        if(FD_ISSET(server_fd, &read_fds)) {
            int new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
            if (new_fd < 0) {
                perror("Accept error");
                continue;
            }

            printf("Client %d connected\n", new_fd);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == -1) {
                    clients[i] = new_fd;
                    break;
                }
            }

        }

        // Go foreach ready client
        for(int i = 0; i < MAX_CLIENTS; i++) {
            int client_fd = clients[i];
            if(client_fd == -1) continue;
            
            if(FD_ISSET(client_fd, &read_fds)) {
                memset(buffer, 0, sizeof(buffer));
                int recv_return = recv(client_fd, buffer, sizeof(buffer), 0);

                if(recv_return <= 0 || strcmp(buffer, QUIT) == 0) {
                    printf("Client %d disconnected\n", client_fd);
                    close(client_fd);
                    clients[i] = -1;
                    continue;
                } 
            
                printf("Recived from client %d: %s\n", client_fd, buffer);

                send(client_fd, buffer, recv_return, 0);
            }
        }

    }

    close(server_fd);

    return 0;
}
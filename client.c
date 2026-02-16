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
#define QUIT ":quit"

int main() {
    // Will store server addr
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    char buffer[BUFFER_SIZE] = {0};

    // Create client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    char message[100] = "";

    while(1) {
        // Get the message the client wants to send to the server
        printf("%d, Message to server: ", client_fd);
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // Replace the new line with a null terminator

        if(strcmp(message, QUIT) == 0) {
            close(client_fd);
            return 0;
        }
        
        // Send data to the server
        send(client_fd, message, strlen(message), 0);
        
        // Read data from the server
        recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        printf("\nServer response: %s\n\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    return 0;
}
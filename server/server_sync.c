#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

void sync_directory(int client_socket) {

    char buffer[MAX_BUFFER_SIZE];

    // Nhận yêu cầu từ client
    ssize_t received_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (received_bytes <= 0) {
        perror("Error receiving data");
        return;
    }

    buffer[received_bytes] = '\0'; // Đảm bảo kết thúc chuỗi

    // Kiểm tra nếu là yêu cầu đóng kết nối
    if (strcmp(buffer, "EXIT") == 0) {
        printf("Client requested to close connection. Closing...\n");
        // Đóng kết nối
        close(client_socket);
    } else {
        // Thực hiện đồng bộ thư mục hoặc xử lý yêu cầu từ client ở đây
        printf("Received request from client: %s\n", buffer);

        // Gửi phản hồi về client
        char response[MAX_BUFFER_SIZE] = "Request received successfully. With content: ";
        strcat(response, buffer);
        send(client_socket, response, strlen(response), 0);
        send(client_socket, buffer, strlen(buffer), 0);
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Config Server's address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind server's address to socket
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_socket, 5) == -1) {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    struct pollfd fds[1];
    fds[0].fd = server_socket;
    fds[0].events = POLLIN; // Check read event

    while (1) {
        // Use poll() to check event on socket server
        int activity = poll(fds, 1, -1);
        if (activity < 0) {
            perror("Error in poll");
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN) {
            // Accept connection from client
            client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
            if (client_socket == -1) {
                perror("Error accepting connection");
            } else {
                printf("Client connected.\n");

                // Process sync function
                sync_directory(client_socket);

                // Remove client socket
                close(client_socket);
            }
        }
    }

    // Remove server socket after all
    close(server_socket);

    return 0;
}

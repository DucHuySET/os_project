#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include "../utils/folder_utils.h"
#include "../include/cJSON.h"

#define PORT 12345
#define MAX_BUFFER_SIZE 1024
#define MAX_JSON_LENGTH 4096
#define GET "GET"
#define PUT "PUT"
#define POST "POST"

void sync_directory(int client_socket);
void process_get(int client_socket);
void clearBuffer(char buffer[MAX_BUFFER_SIZE]);
// void cut_string(int a, int b, char *src);

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

    int count_tmp = 0;
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
            // Update the fds array after accepting a new connection
            fds[1].fd = client_socket;
            fds[1].events = POLLIN;
        }
        if (fds[1].revents & POLLHUP) {
            printf("Client disconnected. Exiting...\n");
            break;
        }
        count_tmp++;
    }

    // Remove server socket after all
    close(server_socket);

    return 0;
}

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
        if(strcmp(buffer, GET)==0){
            process_get(client_socket);
        }
    }
}

void process_get(int client_socket){
    char buffer[MAX_BUFFER_SIZE];
    char json_tmp[MAX_BUFFER_SIZE];
    json_tmp[0]='\0';
    printf("%s\n", json_tmp);
    explore_directory("../test/test_folder_2", json_tmp);
    send(client_socket, json_tmp, strlen(json_tmp), 0);

    //receive list file req from client
    clearBuffer(buffer);
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    clearBuffer(json_tmp);
    strcpy(json_tmp, buffer);
    clearBuffer(buffer);

    cJSON* list_req_file = cJSON_Parse(json_tmp);
    cJSON* item_tmp;
    if (list_req_file == NULL || !cJSON_IsArray(list_req_file)) {
        fprintf(stderr, "Error: Invalid JSON list file req array.\n");
        return ;
    }
    cJSON_ArrayForEach(item_tmp, list_req_file){
        
        cJSON* file_path = cJSON_GetObjectItem(item_tmp, "full_path");
        
        if(cJSON_IsString(file_path) && (file_path->valuestring) != NULL){
            FILE* file = fopen(file_path->valuestring, "r");
            if(!file){
                perror("Error opening file\n");
                exit(EXIT_FAILURE); 
            }
            while (1)
            {   
                size_t byte_read = fread(buffer, 1, MAX_BUFFER_SIZE, file);
                if (byte_read <= 0) {
                    break; // End of file or error
                }
                if (send(client_socket, buffer, byte_read, 0) == -1) {
                    perror("Error sending data\n");
                    fclose(file);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}

void clearBuffer(char buffer[MAX_BUFFER_SIZE]){
    memset(buffer, 0, MAX_BUFFER_SIZE);
}
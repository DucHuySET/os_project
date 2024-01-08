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
#include <stdbool.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024
#define MAX_JSON_LENGTH 4096
#define GET "GET"
#define PUT "PUT"
#define POST "POST"

void sync_directory(int client_socket);
void process_get(int client_socket, char* dest_path);
void process_put(int client_socket, char* dest_path);
void clearBuffer(char buffer[MAX_BUFFER_SIZE]);
void compare_json_obj_to_GET(cJSON* json_Obj_Response, cJSON* json_Obj_Client, cJSON* list_req_file, int* file_count);

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
    clearBuffer(buffer);
    recv(client_socket, buffer, sizeof(buffer), 0);
    char dest_path[MAX_PATH_LENGTH];
    clearBuffer(dest_path);
    strcpy(dest_path, buffer);
    send(client_socket, buffer, sizeof(buffer), 0);

    clearBuffer(buffer);
    recv(client_socket, buffer, sizeof(buffer), 0);

    printf("Received request from client: %s\n", buffer);
    // Gửi phản hồi về client
    char response[MAX_BUFFER_SIZE] = "Request received successfully. With content: ";
    strcat(response, buffer);
    send(client_socket, response, strlen(response), 0);
    if(strcmp(buffer, GET)==0){
        process_get(client_socket, dest_path);
    }else if(strcmp(buffer, PUT) == 0){
        process_put(client_socket, dest_path);
    }else if(strcmp(buffer, POST) == 0){
        process_get(client_socket, dest_path);
        process_put(client_socket, dest_path);
    }
}

void clearBuffer(char buffer[MAX_BUFFER_SIZE]){
    memset(buffer, 0, MAX_BUFFER_SIZE);
}

void process_get(int client_socket, char* dest_path){
    char buffer[MAX_BUFFER_SIZE];
    char json_tmp[MAX_BUFFER_SIZE];
    json_tmp[0]='\0';
    printf("%s\n", json_tmp);
    explore_directory(dest_path, json_tmp);
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

void process_put(int client_socket, char* dest_path){
    char buffer[MAX_BUFFER_SIZE];
    char json_tmp[MAX_BUFFER_SIZE];
    clearBuffer(json_tmp);
    clearBuffer(buffer);
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    strcpy(json_tmp, buffer);
    clearBuffer(buffer);

    char ser_dir[MAX_BUFFER_SIZE];
    explore_directory(dest_path,ser_dir);
    cJSON* json_object_client = cJSON_Parse(json_tmp);
    cJSON* json_object_ser = cJSON_Parse(ser_dir);
    cJSON* json_list_file = cJSON_CreateArray();

    if (json_object_client == NULL || !cJSON_IsArray(json_object_client) || json_object_ser == NULL || !cJSON_IsArray(json_object_ser)) {
        fprintf(stderr, "Error: Invalid JSON array.\n");
        return ;
    }
    int file_req_count = 0;
    compare_json_obj_to_GET(json_object_client, json_object_ser, json_list_file, &file_req_count);
    strcpy(buffer, cJSON_Print(json_list_file));
    send(client_socket, buffer, strlen(buffer), 0);
    int i;
    for(i=0; i<file_req_count; i++){
        size_t bytes_recv;
        bytes_recv = recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_recv <= 0){
            break;
        }
        cJSON* item = cJSON_GetArrayItem(json_list_file, i);
        cJSON* name_file = cJSON_GetObjectItem(item, "name");
        char file_path_name[MAX_PATH_LENGTH] = dest_path;
        strcat(file_path_name, name_file->valuestring);
        FILE* file = fopen(file_path_name, "w");
        if(file == NULL)
        {
            perror("[-]Error in creating file.\n");
            exit(1);
        }
        fprintf(file, "%s", buffer);
        clearBuffer(buffer);
        fclose(file);
    }
}

void compare_json_obj_to_GET(cJSON* json_Obj_Response, cJSON* json_Obj_Client, cJSON* list_req_file, int* file_count){
    bool check = false;
    cJSON *item_res, *item_cli;
    cJSON_ArrayForEach(item_res, json_Obj_Response){
        cJSON* name_res = cJSON_GetObjectItem(item_res, "name");
        cJSON* hash_res = cJSON_GetObjectItem(item_res, "hash");
        if (cJSON_IsString(name_res) && (name_res->valuestring != NULL)) { 
            char* file_name_res = name_res->valuestring;
            // Tim file trong json client
            cJSON_ArrayForEach(item_cli, json_Obj_Client){
                cJSON* name_cli = cJSON_GetObjectItem(item_cli, "name");
                
                if(strcmp(file_name_res, name_cli->valuestring) == 0){
                    cJSON* hash_cli = cJSON_GetObjectItem(item_cli, "hash");
                    
                    if(strcmp(hash_res->valuestring, hash_cli->valuestring) == 0){
                        printf("File %s exist on both.\n", file_name_res);
                        check = true;
                        break;
                    }else{
                        
                        printf("File %s with same name but different.\n", file_name_res);
                        check = true; //TODO: need to get same name file
                    }
                }
            }

            if(!check){
                printf("File %s need client to send.\n", file_name_res);
                cJSON *file_obj = cJSON_CreateObject();
                cJSON_AddStringToObject(file_obj, "name", name_res->valuestring);
                cJSON * full_path_res = cJSON_GetObjectItem(item_res, "full_path"); 
                cJSON_AddStringToObject(file_obj, "full_path", full_path_res->valuestring);
                cJSON_AddItemToArray(list_req_file, file_obj);
                (*file_count)++;
            }
            check =  false;
        } 
    }
}
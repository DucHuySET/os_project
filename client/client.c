#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../utils/folder_utils.h"
#include "../include/cJSON.h"
#include <stdbool.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024
#define MAX_WORDS 50
#define MAX_WORD_LENGTH 100
#define GET "GET"
#define PUT "PUT"
#define POST "POST"


void getUserInput(char user_input[MAX_BUFFER_SIZE], int* retFlag);

void parseInput(char* parsedInput[MAX_WORDS], char user_input[MAX_BUFFER_SIZE]);

void do_get(int client_socket);

void clearBuffer(char buffer[MAX_BUFFER_SIZE]);

void compare_json_obj_to_GET(cJSON* json_Obj_Response, cJSON* json_Obj_Client, cJSON* list_req_file, int* file_count);

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char user_input[MAX_BUFFER_SIZE];
    int count_tmp = 0;
    char* parsedInput[MAX_WORDS];

    // Tạo socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ server
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Kết nối đến server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");
    
    while (1) {
        int retFlag;
        getUserInput(user_input, &retFlag);
        if (retFlag == 2)
            break;

        // Gửi yêu cầu đóng kết nối nếu người dùng nhập "EXIT"
        if (strcmp(user_input, "EXIT") == 0) {
            const char* exit_request = "EXIT";
            send(client_socket, exit_request, strlen(exit_request), 0);
            break;
        }

        parseInput(parsedInput, user_input);

        if (strcmp(parsedInput[2], GET) == 0){
            do_get(client_socket);
        }else if (strcmp(parsedInput[2], PUT) == 0)
        {
            // do_put();
        }else if (strcmp(parsedInput[2], POST) == 0)
        {
            // do_post();
        }else {
            printf("Option not valid, please check again!");
            break;
        }

        // Nhận phản hồi từ server
        // char sync_status[MAX_BUFFER_SIZE];
        // recv(client_socket, sync_status, MAX_BUFFER_SIZE, 0);
        // printf("Server response: %s\n", sync_status);
        
        count_tmp++;
    }

    // Đóng kết nối
    close(client_socket);

    printf("Client closed.\n");

    return 0;
}

void getUserInput(char user_input[MAX_BUFFER_SIZE], int* retFlag)
{
    *retFlag = 1;
    printf("Enter a command: ");

    // Use fgets to read the entire line
    if (fgets(user_input, MAX_BUFFER_SIZE, stdin) == NULL) {
        perror("Error reading input");
        *retFlag = 2;
        return;
    }

    // Remove newline character if present
    size_t len = strlen(user_input);
    if (len > 0 && user_input[len - 1] == '\n')
    {
        user_input[len - 1] = '\0';
    }
}

void parseInput(char* parsedInput[MAX_WORDS], char user_input[MAX_BUFFER_SIZE]){
    int word_count = 0;
    char sentenceCopy[MAX_BUFFER_SIZE];
    strcpy(sentenceCopy, user_input);
    char* token = strtok(sentenceCopy, " ");
    while (token != NULL && word_count < MAX_WORDS) {
        
        parsedInput[word_count] = strdup(token);
        word_count++;
        token = strtok(NULL, " ");
    }
    int i;
}

void do_get(int client_socket){
    char response[MAX_BUFFER_SIZE];
    send(client_socket, GET, sizeof(GET), 0);
    char buffer[MAX_BUFFER_SIZE];
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    printf("%s\n", buffer);
    // Receive directory info from server
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    strcpy(response, buffer);
    clearBuffer(buffer);
    // Create clinet's folder json
    char client_dir[MAX_JSON_BUFFER];
    explore_directory("../test/test_folder_1", client_dir);

    cJSON *json_Obj_Respnse = cJSON_Parse(response);
    cJSON *json_Obj_Client = cJSON_Parse(client_dir);
    cJSON *list_file_request = cJSON_CreateArray();
    int file_req_count = 0;

    if (json_Obj_Respnse == NULL || !cJSON_IsArray(json_Obj_Respnse) || json_Obj_Client == NULL || !cJSON_IsArray(json_Obj_Client)) {
        fprintf(stderr, "Error: Invalid JSON array.\n");
        return ;
    }
    compare_json_obj_to_GET(json_Obj_Respnse, json_Obj_Client, list_file_request, &file_req_count);
    
    char json_tmp[MAX_BUFFER_SIZE];
    strcpy(json_tmp, cJSON_Print(list_file_request));

    send(client_socket, json_tmp, strlen(json_tmp), 0);
    printf("%d", file_req_count);
    int i;
    for(i=0; i<file_req_count; i++){
        size_t bytes_recv;
        bytes_recv = recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes_recv <= 0){
            break;
        }
        cJSON* item = cJSON_GetArrayItem(list_file_request, i);
        cJSON* name_file = cJSON_GetObjectItem(item, "name");
        char file_path_name[MAX_PATH_LENGTH] = "../test/test_folder_1/";
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

void clearBuffer(char buffer[MAX_BUFFER_SIZE]){
    memset(buffer, 0, strlen(buffer));
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
                printf("File %s need to GET.\n", file_name_res);
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
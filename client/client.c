#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include "../utils/folder_utils.h"
#include "../include/cJSON.h"
#include <stdbool.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024
#define MAX_WORDS 50
#define MAX_WORD_LENGTH 100
#define MAX_PATH_LENGTH 1024
#define MAX_IP_LENGTH 40 //IPv6 has max length 39
#define GET "GET"
#define PUT "PUT"
#define POST "POST"


void getUserInput(char user_input[MAX_BUFFER_SIZE], int* retFlag);

void parseInput(char* parsedInput[MAX_WORDS], char user_input[MAX_BUFFER_SIZE]);

void do_get(int client_socket);

void do_put(int client_socket);

void do_post(int client_socket);

void clearBuffer(char buffer[MAX_BUFFER_SIZE]);

void compare_json_obj_to_GET(cJSON* json_Obj_Response, cJSON* json_Obj_Client, cJSON* list_req_file, int* file_count);

void print_syntax();

void parse_dest();

bool is_valid_path(const char *path);

int main(int argc, char* argv[]) {
    if (argc < 4){
        printf("Too few argument, check again!\n");
        print_syntax();
        return 1;
    }else if (argc > 4)
    {
        printf("Too much argument, check again!\n");
        print_syntax();
        return 1;
    }
    //declare and clear input var
    char src_path[MAX_PATH_LENGTH]; memset(src_path, 0, strlen(src_path));
    char dest_ip[MAX_IP_LENGTH]; memset(dest_ip, 0, strlen(dest_ip));
    char dest_path[MAX_PATH_LENGTH]; memset(dest_path, 0, strlen(dest_path));

    if(is_valid_path(argv[1])){
        strcpy(src_path, argv[1]);
    }
    parse_dest(dest_ip, dest_path, argv[2]); 

    int client_socket;
    struct sockaddr_in server_address;
    char user_input[MAX_BUFFER_SIZE];
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
    
    if (strcmp(argv[3], GET) == 0){
        do_get(client_socket);
    }else if (strcmp(argv[3], PUT) == 0)
    {
        do_put(client_socket);
    }else if (strcmp(argv[3], POST) == 0)
    {   
        send(client_socket, POST, sizeof(POST), 0);
        do_get(client_socket);
        do_put(client_socket);
    }else {
        printf("Method not valid, please check again!");
        return 1;
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
}

void clearBuffer(char buffer[MAX_BUFFER_SIZE]){
    memset(buffer, 0, MAX_BUFFER_SIZE);
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

void print_syntax(){
    printf("Right syntax: ./client source_address destination_IP@destination_address method\n");
    printf("GET: one way sync from server\nPUT: one way sync from client\nPOST: two ways sync\n");
}

void parse_dest(char* dest_ip, char* dest_path, char* input){
    bool check = false;
    int i = 0;
    for (i; i<strlen(input); i++){
        if (input[i] == '@'){
            check = true;
            break;
        }
    }
    if(check){
        char input_tmp[MAX_PATH_LENGTH];
        strcpy(input_tmp, input);
        char* token = strtok(input_tmp, "@");
        strcpy(dest_ip, token);
        token = strtok(NULL, "@");
        strcpy(dest_path, token);
    }else{
        strcpy(dest_path, input);
    }
}

bool is_valid_path(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}


void do_get(int client_socket){
    printf("GET start!\n");
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
    printf("GET end!\n");
}

void do_put(int client_socket){
    printf("PUT start!\n");
    send(client_socket, PUT, sizeof(PUT), 0);
    char buffer[MAX_BUFFER_SIZE];
    clearBuffer(buffer);
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    // Create clinet's folder json
    char client_dir[MAX_BUFFER_SIZE];
    explore_directory("../test/test_folder_1", client_dir);
    send(client_socket, client_dir, strlen(client_dir), 0);

    clearBuffer(buffer);
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    char json_tmp[MAX_BUFFER_SIZE];
    clearBuffer(json_tmp);
    strcpy(json_tmp, buffer);
    clearBuffer(buffer);
    printf("%s\n", json_tmp);
    
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
    printf("PUT end!\n");
}
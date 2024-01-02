#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024
#define MAX_WORDS 50
#define MAX_WORD_LENGTH 100
#define GET "GET"
#define PUT "PUT"
#define POST "POST"

void send_file_name(int server_socket, const char *file_name);

void getUserInput(char user_input[MAX_BUFFER_SIZE], int* retFlag);

void parseInput(char* parsedInput[MAX_WORDS], char user_input[MAX_BUFFER_SIZE]);

void do_get(int client_socket);

void clearBuffer(char buffer[MAX_BUFFER_SIZE]);

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
    send(client_socket, GET, sizeof(GET), 0);
    char buffer[MAX_BUFFER_SIZE];
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    printf("%s\n", buffer);
    clearBuffer(buffer);
    recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    printf("%s\n", buffer);
}

void clearBuffer(char buffer[MAX_BUFFER_SIZE]){
    memset(buffer, 0, strlen(buffer));
}

void send_file_name(int server_socket, const char* file_name) {
    // Gửi tên file đến server
    send(server_socket, file_name, strlen(file_name), 0);
}
#define _XOPEN_SOURCE
#include"folder_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/sha.h> 
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>

#include "../include/cJSON.h"

// Calculate SHA_256 for file's content only
int calculate_sha256(const char *file_path, unsigned char hash[SHA256_DIGEST_LENGTH]) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    EVP_MD_CTX *mdctx;
    const EVP_MD *md = EVP_sha256();  // Use the SHA-256 message digest algorithm
    mdctx = EVP_MD_CTX_new();// mdctx-massage digest context

    if (!mdctx) {
        perror("Error creating message digest context");
        fclose(file);
        return -1;
    }

    EVP_DigestInit_ex(mdctx, md, NULL);

    size_t bytesRead;
    unsigned char buffer[SHA_BUFFER_SIZE];
    // read content
    while ((bytesRead = fread(buffer, 1, SHA_BUFFER_SIZE, file)) != 0) {
        EVP_DigestUpdate(mdctx, buffer, bytesRead);
    }

    EVP_DigestFinal_ex(mdctx, hash, NULL);
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    return 0;
}

void explore_directory(const char *path, char *jsonOutput){
    DIR *dir;
    struct dirent *entry;

    cJSON *json_array = cJSON_CreateArray();

    struct stat file_stat;

    if((dir = opendir(path)) != NULL){ //open directory
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char full_path[MAX_PATH_LENGTH];
                sprintf(full_path, "%s/%s", path, entry->d_name);

                unsigned char hash[SHA256_DIGEST_LENGTH];
                calculate_sha256(full_path, hash);
                
                if(entry->d_type == FILE_T){
                    if (stat(full_path, &file_stat) != 0) {
                        perror("Error getting information for file");
                        return ;
                    }
                    char timeStr[MAX_NAME_LENGTH];
                    strftime(timeStr, sizeof(timeStr), pattern_1, localtime(&file_stat.st_mtime));
                    strcpy(timeStr, time_to_string(&file_stat.st_mtime));

                    cJSON *file_obj = cJSON_CreateObject();
                    cJSON_AddStringToObject(file_obj, "name", entry->d_name);
                    cJSON_AddStringToObject(file_obj, "type", "file");
                    cJSON_AddStringToObject(file_obj, "hash", hash_to_string(hash));
                    cJSON_AddStringToObject(file_obj, "full_path", full_path);
                    cJSON_AddStringToObject(file_obj, "m_time", timeStr);
                    cJSON_AddItemToArray(json_array, file_obj);
                }
            }
        }
        
    }

    strcpy(jsonOutput, cJSON_Print(json_array));

    closedir(dir);
}

char *hash_to_string(unsigned char hash[SHA256_DIGEST_LENGTH]) {
    char *hash_str = (char *)malloc(SHA256_DIGEST_HEX_LENGTH + 1);
    if (!hash_str) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hash_str[i * 2], "%02x", hash[i]);
    }

    hash_str[SHA256_DIGEST_HEX_LENGTH] = '\0';
    return hash_str;
}

void path_join(char * path, char* adding, char* result){

}

char* time_to_string(time_t* time_info){
    char* timeStr = (char *)malloc(MAX_PATH_LENGTH + 1);
    strftime(timeStr, MAX_PATH_LENGTH, pattern_1, localtime(time_info));
    return timeStr;
}

time_t string_to_time(char* timeStr){
    struct tm tm_struct;
    time_t time_info;
    strptime(timeStr, pattern_1, &tm_struct);
    time_info =  mktime(&tm_struct);
    return time_info;
}

#ifndef FOLDER_UTILS_H
#define FOLDER_UTILS_H

#include <openssl/sha.h> 
#include <openssl/evp.h>

#define MAX_NAME_LENGTH 100
#define SHA_BUFFER_SIZE 4096
#define MAX_FILE_QUANTITY 100
#define MAX_FOLDER_QUANTITY 50
#define MAX_PATH_LENGTH 1024

typedef struct file_struc{
    char name[MAX_NAME_LENGTH];
    unsigned char hash_Content[SHA256_DIGEST_LENGTH];
    char full_path[MAX_PATH_LENGTH];
} file_struc;

typedef struct folder_struc{
    int file_count;
    int subfolder_count;
    file_struc * listFiles[MAX_FILE_QUANTITY];
    struct folder_struc * subfolder[MAX_FOLDER_QUANTITY];
} folder_struc;

int calculate_sha256(const char *file_path, unsigned char hash[SHA256_DIGEST_LENGTH]);
int 

#endif
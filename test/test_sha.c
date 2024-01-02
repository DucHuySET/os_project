#include <stdio.h>
#include <string.h>
#include "../utils/folder_utils.h"  // Include your utility functions
#include <openssl/sha.h>


int main(){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    calculate_sha256("../client/cli_storage/test_sha.txt", hash);
    printf("SHA-256 Hash: ");
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
    return 0;
}
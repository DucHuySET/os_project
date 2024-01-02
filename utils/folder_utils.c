#include"folder_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/sha.h> 

int calculate_sha256(const char *file_path, unsigned char hash[SHA256_DIGEST_LENGTH]) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    EVP_MD_CTX *mdctx;
    const EVP_MD *md = EVP_sha256();  // Use the SHA-256 message digest algorithm
    mdctx = EVP_MD_CTX_new();

    if (!mdctx) {
        perror("Error creating message digest context");
        fclose(file);
        return -1;
    }

    EVP_DigestInit_ex(mdctx, md, NULL);

    size_t bytesRead;
    unsigned char buffer[SHA_BUFFER_SIZE];

    while ((bytesRead = fread(buffer, 1, SHA_BUFFER_SIZE, file)) != 0) {
        EVP_DigestUpdate(mdctx, buffer, bytesRead);
    }

    EVP_DigestFinal_ex(mdctx, hash, NULL);
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    return 0;
}

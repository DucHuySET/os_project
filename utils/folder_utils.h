/** identify folder or file following document in dirent.h
 *  Enum {
    DT_UNKNOWN = 0,
# define DT_UNKNOWN	DT_UNKNOWN
    DT_FIFO = 1,
# define DT_FIFO	DT_FIFO
    DT_CHR = 2,
# define DT_CHR		DT_CHR
    DT_DIR = 4,
# define DT_DIR		DT_DIR directory
    DT_BLK = 6,
# define DT_BLK		DT_BLK
    DT_REG = 8,
# define DT_REG		DT_REG regular file
    DT_LNK = 10,
# define DT_LNK		DT_LNK
    DT_SOCK = 12,
# define DT_SOCK	DT_SOCK
    DT_WHT = 14
# define DT_WHT		DT_WHT
  };
*/


#ifndef FOLDER_UTILS_H
#define FOLDER_UTILS_H

#include <openssl/sha.h> 
#include <openssl/evp.h>

#define MAX_NAME_LENGTH 100
#define SHA_BUFFER_SIZE 4096
#define MAX_FILE_QUANTITY 100
#define MAX_FOLDER_QUANTITY 50
#define MAX_PATH_LENGTH 1024
#define MAX_JSON_BUFFER 4096
#define SHA256_DIGEST_HEX_LENGTH (SHA256_DIGEST_LENGTH * 2)
#define pattern_1 "%Y-%m-%d %H:%M:%S"

# define DIR_T	4
# define FILE_T 8

#define _XOPEN_SOURCE

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
void explore_directory(const char *path, char *jsonOutput);
char *hash_to_string(unsigned char hash[SHA256_DIGEST_LENGTH]);\
char* time_to_string(time_t* time_info);
time_t string_to_time(char* timeStr);
#endif
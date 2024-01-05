#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAX_PATH_LENGTH 1024

# define DIR_T	4
# define FILE_T 8



typedef struct {
    char name[MAX_PATH_LENGTH];
    char full_path[MAX_PATH_LENGTH];
} DistinguishedItem;

void explore_directory(const char *path, int indent, char *jsonOutput, DistinguishedItem *distinguishedItems, int *count);
bool compare_folders(const char *json1, const char *json2, DistinguishedItem *distinguishedItems, int *count);
void print_distinguished_to_json(const char *outputPath, DistinguishedItem *distinguishedItems, int count);

int main() {
    const char *folder1 = "./test_folder_1";
    const char *folder2 = "./test_folder_2";

    char jsonOutput1[4096] = "";  // Adjust the size 
    char jsonOutput2[4096] = "";

    DistinguishedItem distinguishedItems[1024];  // Adjust the size 
    int count = 0;

    explore_directory(folder1, 0, jsonOutput1, distinguishedItems, &count);
    explore_directory(folder2, 0, jsonOutput2, distinguishedItems, &count);
    printf("%s\n", jsonOutput1);
    printf("/////////////////////\n");
    printf("%s\n", jsonOutput2);

    if (compare_folders(jsonOutput1, jsonOutput2, distinguishedItems, &count)) {
        printf("Folders are identical.\n");
        print_distinguished_to_json("distinguished_items.json", distinguishedItems, count);
    } else {
        printf("Folders are different. Distinguished items have been saved to 'distinguished_items.json'.\n");
        print_distinguished_to_json("distinguished_items.json", distinguishedItems, count);
    }

    return 0;
}

void explore_directory(const char *path, int indent, char *jsonOutput, DistinguishedItem *distinguishedItems, int *count) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;

    char jsonOutput_tmp[4096];

    if ((dir = opendir(path)) == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            if(entry->d_type == 8){
                sprintf(jsonOutput_tmp, "{\n\t\"name\":\"%s\",\n\t\"type\":\"%d\"\n}\n", entry->d_name, entry->d_type);
            strcat(jsonOutput, jsonOutput_tmp);
            }
            
        }
    }

    closedir(dir);
}

bool compare_folders(const char *json1, const char *json2, DistinguishedItem *distinguishedItems, int *count) {
    if (strcmp(json1, json2) == 0) {
        return true;  // Folders are identical
    }

    // Save distinguished subfolder
    strcpy(distinguishedItems[*count].name, "root");
    strcpy(distinguishedItems[*count].full_path, "");
    (*count)++;

    return false;  // Folders are different
}

void print_distinguished_to_json(const char *outputPath, DistinguishedItem *distinguishedItems, int count) {
    FILE *outputFile = fopen(outputPath, "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    fprintf(outputFile, "[\n");
    for (int i = 0; i < count; i++) {
        // Ensure sufficient space in the output file
        if (fprintf(outputFile, "  {\"name\": \"%s\", \"full_path\": \"%s\"}", distinguishedItems[i].name, distinguishedItems[i].full_path) < 0) {
            fprintf(stderr, "Error writing to output file.\n");
            exit(EXIT_FAILURE);
        }

        if (i < count - 1) {
            // Ensure sufficient space in the output file
            if (fprintf(outputFile, ",") < 0) {
                fprintf(stderr, "Error writing to output file.\n");
                exit(EXIT_FAILURE);
            }
        }

        // Ensure sufficient space in the output file
        if (fprintf(outputFile, "\n") < 0) {
            fprintf(stderr, "Error writing to output file.\n");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(outputFile, "]\n");

    fclose(outputFile);
}

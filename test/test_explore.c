#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

#define MAX_PATH_LENGTH 256

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

    char jsonOutput1[4096];  // Adjust the size based on your needs
    char jsonOutput2[4096];

    DistinguishedItem distinguishedItems[1024];  // Adjust the size based on your needs
    int count = 0;

    explore_directory(folder1, 0, jsonOutput1, distinguishedItems, &count);
    explore_directory(folder2, 0, jsonOutput2, distinguishedItems, &count);

    if (compare_folders(jsonOutput1, jsonOutput2, distinguishedItems, &count)) {
        printf("Folders are identical.\n");
        printf("Folders are different. Distinguished items have been saved to 'distinguished_items.json'.\n");
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

    if ((dir = opendir(path)) == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            for (int i = 0; i < indent; i++) {
                if (snprintf(jsonOutput, 4096, "  ") < 0) {
                    fprintf(stderr, "Error: Buffer overflow.\n");
                    exit(EXIT_FAILURE);
                }
            }

            char full_path[MAX_PATH_LENGTH];
            snprintf(full_path, sizeof(full_path)+1, "%s/%s", path, entry->d_name);
            printf("%s\n", full_path);

            if (stat(full_path, &file_stat) == -1) {
                perror("Error getting file stat");
                exit(EXIT_FAILURE);
            }

            if (snprintf(jsonOutput, 4096, "{\n") < 0) {
                fprintf(stderr, "Error: Buffer overflow.\n");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < indent + 1; i++) {
                if (snprintf(jsonOutput, 4096, "  ") < 0) {
                    fprintf(stderr, "Error: Buffer overflow.\n");
                    exit(EXIT_FAILURE);
                }
            }

            if (snprintf(jsonOutput, 4096, "\"name\": \"%s\", \"type\": ", entry->d_name) < 0) {
                fprintf(stderr, "Error: Buffer overflow.\n");
                exit(EXIT_FAILURE);
            }

            if (S_ISDIR(file_stat.st_mode)) {
                if (snprintf(jsonOutput, 4096, "\"folder\",\n") < 0) {
                    fprintf(stderr, "Error: Buffer overflow.\n");
                    exit(EXIT_FAILURE);
                }
                explore_directory(full_path, indent + 1, jsonOutput, distinguishedItems, count);
            } else {
                if (snprintf(jsonOutput, 4096, "\"file\"\n") < 0) {
                    fprintf(stderr, "Error: Buffer overflow.\n");
                    exit(EXIT_FAILURE);
                }
                // Save distinguished file
                strcpy(distinguishedItems[*count].name, entry->d_name);
                strcpy(distinguishedItems[*count].full_path, full_path);
                (*count)++;
            }

            for (int i = 0; i < indent; i++) {
                if (snprintf(jsonOutput, 4096, "  ") < 0) {
                    fprintf(stderr, "Error: Buffer overflow.\n");
                    exit(EXIT_FAILURE);
                }
            }

            if (snprintf(jsonOutput, 4096, "}\n") < 0) {
                fprintf(stderr, "Error: Buffer overflow.\n");
                exit(EXIT_FAILURE);
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

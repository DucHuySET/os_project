#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#define pattern_1 "%Y-%m-%d %H:%M:%S"
#define MAX_PATH_LENGTH 1024

char* time_to_string(time_t* time_info){
    char* timeStr = (char *)malloc(MAX_PATH_LENGTH + 1);
    strftime(timeStr, MAX_PATH_LENGTH, pattern_1, localtime(time_info));
    return timeStr;
}

// Hàm so sánh thời gian chỉnh sửa giữa hai file
int compare_file_modified_times(const char *file1, const char *file2) {
    struct stat st1, st2;

    // Lấy thông tin thời gian chỉnh sửa của file1
    if (stat(file1, &st1) != 0) {
        perror("Error getting information for file1");
        return -1;
    }

    // Lấy thông tin thời gian chỉnh sửa của file2
    if (stat(file2, &st2) != 0) {
        perror("Error getting information for file2");
        return -1;
    }

    // So sánh thời gian chỉnh sửa của hai file
    if (st1.st_mtime > st2.st_mtime) {
        printf("%s is more recently modified than %s\n", file1, file2);
        return 1;
    } else if (st1.st_mtime < st2.st_mtime) {
        printf("%s is more recently modified than %s\n", file2, file1);
        return -1;
    } else {
        printf("%s and %s have the same modification time\n", file1, file2);
        return 0;
    }
}

void get_file_modified_time(const char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0) {
        // Chuyển đổi thời gian chỉnh sửa từ giây thành cấu trúc tm
        struct tm *modified_time = localtime(&st.st_mtime);
        char timeStr[100];
        strftime(timeStr, sizeof(timeStr),"%Y-%m-%d %H:%M:%S",localtime(&st.st_mtime));
        printf("%s\n", timeStr);



        // Hiển thị thời gian chỉnh sửa
        // printf("File modified time: %02d-%02d-%04d %02d:%02d:%02d\n",
        //        modified_time->tm_mday, modified_time->tm_mon + 1, modified_time->tm_year + 1900,
        //        modified_time->tm_hour, modified_time->tm_min, modified_time->tm_sec);
        printf("%s\n", time_to_string(&st.st_mtime));
    } else {
        perror("Error getting file information");
    }
}

int main() {
    const char *file1 = "./test_time.c";  // Thay thế đường dẫn và tên file cụ thể của bạn
    const char *file2 = "./test_sha.c";  // Thay thế đường dẫn và tên file cụ thể của bạn

    int result = compare_file_modified_times(file1, file2);

    get_file_modified_time(file1);
    get_file_modified_time(file2);

    return 0;
}

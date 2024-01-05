#include <stdio.h>
char src[] = "/home/duchuyset/Desktop/Doc_Stu/HDH/project/test/test_folder_2/test_sha cy_2_1.txt";
char des[] = "/home/duchuyset/Desktop/Doc_Stu/HDH/project";

int main(){
    coppy_file(des, strlen(src), src);
    return 0;
}
void coppy_file(char *des, int n, char *src) {
    char filename[1000];
    char des1[1000];
    int k = 0;
    FILE * f1 = fopen(src, "r");
    for (int i = 0; i < strlen(n); i++) {
        if (des[i] == "/") {
            k = 0;
        }
        else {
            filename[k] = des[i];
            k++;
        }
    }
    sscanf(des1, "%s/%s", src, filename);
    FILE *f2 = fopen(des1, "w+");
    char c = fgetc(f1);
    while (c != EOF) {
        fputs(c, f2);
        c = fgetc(f1);
    }
}
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#define MAX_WORD_LENGTH 100

int main(){
    char res[MAX_WORD_LENGTH] = "test ";
    char buffer[100];
    buffer[0] = 'a';
    strcat(res, buffer);
    printf("%s", res);
    return 0;
}
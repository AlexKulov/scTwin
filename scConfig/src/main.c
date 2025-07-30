#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cJSON.h"

/*
 * read file to char *
 */
static char* fileRead(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    uint32_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    while (file != NULL){
        /* get the length */
        if (fseek(file, 0, SEEK_END) != 0){
            break;
        }
        length = ftell(file);
        if (length < 0){
            break;
        }
        if (fseek(file, 0, SEEK_SET) != 0){
            break;
        }
        /* allocate content buffer */
        content = (char*)malloc((size_t)length + sizeof(""));
        if (content == NULL){
            break;
        }
        /* read the file into memory */
        read_chars = fread(content, sizeof(char), (size_t)length, file);
        if ((long)read_chars != length){
            free(content);
            content = NULL;
            break;
        }
        content[read_chars] = '\0';
        break;
    }

    if(file != NULL){
        fclose(file);
    }

    return content;
}

int main(int32_t argc, char** argv){

    char * jsonExample = fileRead("acos/dev/trackers.json");
    cJSON * test = cJSON_Parse(jsonExample);

    printf("%s", jsonExample);
    return 0;
}

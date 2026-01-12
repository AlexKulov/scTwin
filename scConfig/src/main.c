#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cJSON.h"

#include "../sctwin.h"
/*
 * read file to char *
 */
char* fileRead(const char *filename) {
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

static long fileCopy(const char *srcFileName, const char *dstFileName){
    char * srcText = fileRead(srcFileName);
    if(srcText){
        FILE * dstFile = fopen(dstFileName,"w");
        fprintf(dstFile, srcText);
        fclose(dstFile);
        return 1;
    }
    else{
        printf("fileCopy: srcFile=%s not exidt!\n", srcFileName);
    }
    return 0;
}

void system2Txt42(char * scLabel, char * systemName, char * tempName, char * outTxtName){
    string  filePath;
    sprintf(filePath, "./%s/%s.json", systemName, scLabel);
    char * scJsonFile = fileRead(filePath);
    if(scJsonFile){//1.1 если файл json существует
        //2. Парсим json
        cJSON * scJson = cJSON_Parse(scJsonFile);
        //3. Копируем шаблон с нужным именем
        string  tempFile;
        sprintf(tempFile, "./out42/%s.txt", tempName);
        string  out42File;
        sprintf(out42File, "./out42/%s.txt" , outTxtName);
        scJson2Txt42(scJson, tempFile, out42File, systemName);
    }
    else
        printf("Json file name is incorrect = %s\n", filePath);
}

extern void testGenConfigs();
int main(int32_t argc, char** argv){
    testGenConfigs();
    //1. При запуске вводим имя файла, который хотим сконфигурировать
    if(argc>0){
        string out42File = {0};

        sprintf(out42File, "SC_%s" , argv[1]);
        system2Txt42(argv[1], "acos", "SC_Template", out42File);

        sprintf(out42File, "SPS_%s" , argv[1]);
        system2Txt42(argv[1], "sps", "SPS_Simplest", out42File);
    }
    else
        printf("Please, enter json file name\n");

    return 0;
}

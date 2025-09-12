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

extern void scJson2Txt42(cJSON * scJson,
                         char * tempName, char * outName);
int main(int32_t argc, char** argv){

    //1. При запуске вводим имя файла, который хотим сконфигурировать
    if(argc>0){
        string  filePath;
        sprintf(filePath, "./acos/%s.json", argv[1]);
        char * scJsonFile = fileRead(filePath); //"acos/dev/trackers.json"

        if(scJsonFile){//1.1 если файл json существует
            //2. Копируем шаблон с нужным именем
            string pathFrom, pathTo;
            sprintf(pathFrom, "./out42/SC_Template.txt");
            sprintf(pathTo  , "./out42/SC_%s.txt", argv[1]);

            //long
            //wasCopy = fileCopy(pathFrom, pathTo);

            //3. Парсим json
            cJSON * scJson = cJSON_Parse(scJsonFile);
            FILE * sc42File = fopen(pathTo, "r");
            //scJson2Txt42(scJson, sc42File);
            scJson2Txt42(scJson, pathFrom, pathTo);
            fclose(sc42File);


            //printf("%s", jsonExample);
        }
        else
            printf("Json file name is incorrect = %s\n",argv[1]);
    }
    else
        printf("Please, enter json file name\n");

    return 0;
}

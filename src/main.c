#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cJSON.h"

#ifndef FALSE
    #define FALSE (0)
    #define TRUE (1)
#endif

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

#ifdef WIN32
/* #include <winbase.h>*/
#elif

#endif

static long fileCopy(const char *srcFileName, const char *dstFileName){
    char * srcText = fileRead(srcFileName);
    if(srcText){
        FILE * dstFile = fopen(dstFileName,"w");
        fprintf(dstFile, srcText);
        fclose(dstFile);
        return 0;
    }
    else{
        printf("fileCopy: srcFile=%s not exidt!\n", srcFileName);
    }
    return 1;
}

static void briefShowBaseAll(cJSON * configs, uint32_t configSize){

}

static void execSingleBaseConfig(cJSON * configs, uint32_t configSize, uint32_t idNum){
    cJSON * curConfig = NULL;
    for(uint32_t i=0;i<configSize;i++){
        //Satellite sat={0};
        curConfig = cJSON_GetArrayItem(configs, i);
        cJSON * jID = cJSON_GetObjectItem(curConfig, "ID");
        int32_t curID = cJSON_GetNumberValue(jID);
        if((uint32_t)curID == idNum){
            break;
        }
        else{
            curConfig = NULL;
        }
    }
    if(!curConfig){
        printf("Have't config with ID=%i", idNum);
        exit(1);
    }

    /* 2. Форминеум соответствующую папку ИД - ConfigInOut */
    /*CopyFile(..., ..., 0/1): 0-перезапись, 1-только новый*/
    cJSON * param = cJSON_GetObjectItem(curConfig, "SC");
    char name[256];
    strcpy(name, cJSON_GetStringValue(param));
    char pathFrom[256], pathTo[256];
    sprintf(pathFrom, "./scConfig/out42/SC_%s.txt", name);
    sprintf(pathTo, "./configInOut/SC_%s.txt", name);
    long
    copyRes = fileCopy(pathFrom, pathTo);

    param = cJSON_GetObjectItem(curConfig, "Orb");
    strcpy(name, cJSON_GetStringValue(param));
    sprintf(pathFrom, "./scConfig/out42/Orb_%s.txt", name);
    sprintf(pathTo, "./configInOut/Orb_%s.txt", name);
    copyRes = fileCopy(pathFrom, pathTo);

    /* 3. Изменяем twin42.pro */


    /* 4. Запускаем cmake и make */
    system("dir");
    char cmakePath[100] = "../42support";
    char buildPath[100] = "../42support/build";
    char winCmd[254];
    sprintf(winCmd,
            "cmake -DEMULATOR=1 -G \"MinGW Makefiles\" -B %s %s ",
            buildPath, cmakePath);
    system(winCmd);
    sprintf(winCmd,
            "mingw32-make -C %s", buildPath);
    system(winCmd);
    system("..\\42\\42twin.exe configInOut ..\\42\\Model");
    return;
}

int main(int32_t argc, char** argv){

    long isID = FALSE;
    char * ID = NULL;
    uint32_t idNum = 0;
    if(argc>1){
        for(int i=1;i<argc;i++){
            ID = strstr(argv[i],"ID");
            if(ID && ID[2] == '='){
                idNum = atoi(&ID[3]);
                isID = TRUE;
                break;
            }
        }
    }
    /* 1. Считываем файл конфигурации */
    char * jsonText = fileRead("twBase.json");
    cJSON * twBase = cJSON_Parse(jsonText);
    cJSON * configs = cJSON_GetObjectItem(twBase, "configs");
    uint32_t configSize = cJSON_GetArraySize(configs);
    //char * resTest = cJSON_Print(twBase);
    //printf("%s", resTest);

    if(isID){
        execSingleBaseConfig(configs, configSize, idNum);
    }
    else{
        briefShowBaseAll(configs, configSize);
    }

    return 0;
}

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

int main(int32_t argc, char** argv){
    /* 1. Считываем файл конфигурации */
    char * jsonText = fileRead("configs.json");
    cJSON * cJsonExample = cJSON_Parse(jsonText);
    char * resTest = cJSON_Print(cJsonExample);
    /*printf("%s", resTest);*/

    /* 2. Форминеум соответствующую папку ИД - ConfigInOut */
    /*CopyFile(..., ..., 0/1): 0-перезапись, 1-только новый*/
    long
    copyRes = fileCopy("./scConfig/out42/SC_2Whl.txt",
                       "./configInOut/SC_2Whl.txt");
    copyRes = fileCopy("./scConfig/out42/Orb_LEO.txt",
                       "./configInOut/Orb_LEO.txt");

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

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cJSON.h"

#include "../sctwin.h"

#define DESC_STR_MAX_42 (64)
#define MAX_SC_PARAMS (20)
#define STR_BUF_SIZE (2048)
static string scParams[MAX_SC_PARAMS];
const
static string devNameList[MAX_SC_PARAMS] = {
    "wheel",
    "MTB",
    "Thr",
    "Gyro",
    "Mag", //Magnetometer
    "Coarse",//Coarse Sun Sensor
    "Fine",//Fine Sun Sensor
    "Star",//Star Tracker
    "Accel", //Accelerometer
    "Fine Guidance Sensor" //Fine Guidance Sensor
};

static string eqCurList[MAX_SC_PARAMS];

#define N_ACT (3)
static string actNames[N_ACT] = {
    "wheel"
    "MTB",
    "Thr"
};
static uint8_t actFlag[N_ACT];

#define N_SNS (7)
string snsNames[N_SNS] = {
    "Gyro",
    "Mag", //Magnetometer
    "Coarse",//Coarse Sun Sensor
    "Fine Sun Sensor",//Fine Sun Sensor
    "Star",//Star Tracker
    "Accel", //Accelerometer
    "Fine Guidance Sensor" //Fine Guidance Sensor
};

typedef enum{
    NOTHING = 0,
    SC_PARAM,
    ACTUATOR,
    SENSOR,
    ERROR
}PARAM_TYPE;

//возвращает номер
static string * checkNames(const char * str, string * arNames, uint8_t nAr){
    char testStr[STR_SIZE] = {0};
    for(uint8_t i=0; i<nAr; i++){
        strcpy(testStr, arNames[i]);
        if(arNames[i][0]==0){
            break;
        }
        else if(strstr(str, arNames[i]) ||
                strstr(arNames[i], str)){
            return &arNames[i];
        }

    }
    return NULL;
}


typedef struct EqDataBase{
    string  name   [MAX_SC_PARAMS];
    cJSON * json   [MAX_SC_PARAMS];
    cJSON * devJson[MAX_SC_PARAMS];
    uint8_t size;
}EqDataBase;

cJSON * getJsonByName(const cJSON * json, const string name){
    if(!json || !name){
        printf("getJsonByName: json=%p name=%p\n", json, name);
        return NULL;
    }
    cJSON * curJson = json->child;
    for(uint16_t i=0; curJson !=NULL &&
                      i<MAX_SC_PARAMS; i++){
        if(strstr(curJson->string, name)){
            return curJson;
        }
        curJson = curJson->next;
    }
    return NULL;
}

static uint8_t getDbIndex(const EqDataBase * eqDB, string devName){
    //...0 находим номер в DB
    for(uint8_t i=0;i<eqDB->size;i++){
        if(strstr(eqDB->name[i], devName)){
            return i;
        }
    }
    return 0;
}

static uint8_t getCommonDevNum(const EqDataBase * eqDB, string devName){
    uint8_t ind = getDbIndex(eqDB, devName);

    cJSON * blocks = cJSON_GetObjectItem(eqDB->json[ind], "blocks");
    uint8_t blockSize = (uint8_t)cJSON_GetArraySize(blocks);
    cJSON * curBlock = NULL;
    uint8_t commonNum = 0;
    uint8_t blockNum = 0;
    for(uint8_t i=0;i<blockSize;i++){
        curBlock = cJSON_GetArrayItem(blocks, i);
        blockNum = cJSON_GetNumberValue(cJSON_GetObjectItem(curBlock, "n"));
        commonNum += blockNum;
    }
    return commonNum;
}

static void setIntParam(char * newBuf, uint32_t iParam){
    char param[STR_BUF_SIZE/2]={0};
    char desc[STR_BUF_SIZE/2]={0};
    sscanf(newBuf, "%[^!]!%[^\n]", param, desc);
    sprintf(newBuf, "%i ! %s\n", iParam, desc);
}

static void setDblParam(char * newBuf, double dParam){
    char param[STR_BUF_SIZE/2], desc[STR_BUF_SIZE/2];
    sscanf(newBuf, "%s!%s", param, desc);
    sprintf(param,"%lf ", dParam);
    sprintf(newBuf, "%s !%s", param, desc);
}

static void setVecParam(char * newBuf, const double vParam[3]){
    char param[STR_BUF_SIZE/2], desc[STR_BUF_SIZE/2];
    sscanf(newBuf, "%s!%s", param, desc);
    sprintf(param,"%lf %lf %lf ", vParam[0], vParam[1], vParam[2]);
    sprintf(newBuf, "%s !%s", param, desc);
}

#define STR(x) #x
static void setStrParam(char * newBuf, const char * str){
    char param[STR_BUF_SIZE/2]={0};
    char desc[STR_BUF_SIZE/2]={0};
    sscanf(newBuf, "%[^!]!%[^\n]", param, desc);
    sprintf(newBuf, "%s ! %s\n", str, desc);
}

static void setDevicePosition(char * strAxis, cJSON * position){

}

static char * jsonPrint(const cJSON * json){
    char * jStr = cJSON_Print(json);
    uint8_t strLen=0;
    if(cJSON_IsString(json)){
        strLen = strlen(jStr);
        jStr[strLen-1] = 0;
        jStr++;
    }
    return jStr;
}

static char * devicePrint(char * nextStrNewBuf, const char * curDevName, uint8_t jDev,
                        const string * devBuf, uint8_t devBufSize){
    sprintf(nextStrNewBuf,
    "=============================== %s %i "
    "===================================\n",
    curDevName, jDev);
    nextStrNewBuf = strstr(nextStrNewBuf, "\n");
    nextStrNewBuf++;

    for(uint8_t k=0; k<devBufSize; k++){
        sprintf(nextStrNewBuf, "%s", devBuf[k]);
        nextStrNewBuf = strstr(nextStrNewBuf, "\n");
        nextStrNewBuf++;
    }
    return nextStrNewBuf;
}


extern char* fileRead(const char *filename);
void scJson2Txt42(cJSON * scJson, char * tempName, char * outName){
    FILE * temp = fopen(tempName, "r");
    FILE * out  = fopen(outName,  "w");
    if(temp && out && scJson){
        //1. Открываем файл шаблона и начинаем его переписывать в выход.
        cJSON * curParam = scJson->child;
        uint16_t nParams = 0;
        string * pStr = NULL;
        EqDataBase eqDB = {0};
        char curParamName[STR_SIZE] = {0};
        char curDevName[STR_SIZE] = {0};
        //2. заполняем базу аппаратуры
        for(uint16_t i=0; curParam !=NULL &&
                          nParams<MAX_SC_PARAMS; i++){
            strcpy(scParams[i],curParam->string);
            pStr = checkNames(scParams[i], devNameList, MAX_SC_PARAMS);
            if(pStr){//2.1 записываем Имя/Json/Файл
                strcpy(curDevName, *pStr);
                uint8_t size = eqDB.size;
                strcpy(eqDB.name[size], curDevName);
                eqDB.json[size] = curParam;
                string devFileName = {0};
                sprintf(devFileName, "./acos/dev/%s.json", curDevName);
                char * scJsonFile = fileRead(devFileName);
                cJSON * devJson = cJSON_Parse(scJsonFile);
                eqDB.devJson[size] = devJson;
                eqDB.size++;
            }
            curParam = curParam->next;
            nParams++;
        }

        char curBuf[STR_BUF_SIZE]={0};
        char newBuf[STR_BUF_SIZE]={0};
        //int endOfFile =
        fgets(curBuf, STR_BUF_SIZE, temp);
        //3. запускаем построчное чтение
        while (fgets(newBuf, STR_BUF_SIZE, temp) != NULL) {
            //4. Проверка, если строка из файла содержит любой параметр
            pStr = checkNames(newBuf, scParams, nParams);
            if(pStr){
                strcpy(curParamName, *pStr);
                //4.1 Проверка, если параметр содержит строку аппаратуры
                pStr = checkNames(*pStr, eqDB.name, eqDB.size);
                if(pStr){//заполняем newBuf!!! если попался Number of ...
                    strcpy(curDevName, *pStr);
                    //...1 считаем кол-во аппаратуры данного типа
                    uint8_t nEq = getCommonDevNum(&eqDB, pStr);
                    //...2 модифицируем строку с кол-вом аппаратуры
                    setIntParam(newBuf, nEq);
                    //...3 считать строку с индексом 0 прибора в буфер 0
                    char buf0[128]={0};
                    fscanf(temp, "%128[^\n]\n", buf0);
                    //...4 считать строки типового описания в буфер1
                    //char devBuf[DESC_STR_MAX_42][128]={0};
                    static string devBuf[MAX_SC_PARAMS]={0};
                    uint8_t devBufSize=0;
                    char newDevSection[128]={0};
                    for(uint8_t i=0;i<MAX_SC_PARAMS;i++){
                        //.....построчная запись в буфер
                        fgets(devBuf[i], STR_SIZE, temp);
                        if((strstr(devBuf[i], "*****") ||
                            strstr(devBuf[i], "=====") )){
                            devBufSize = i;
                            if(strstr(devBuf[i], "*****")){
                               strcpy(newDevSection, devBuf[i]);
                            }
                            break;
                        }
                    }

                    ///? //...4.1 если после считывания не начался новый прибор, то дойти до нового
                    //...5 цикл по типам приборов
                    cJSON * NumberOfDevice = getJsonByName(scJson, curDevName);
                    cJSON * blocks = getJsonByName(NumberOfDevice, "blocks");
                    uint8_t blockSize = (uint8_t)cJSON_GetArraySize(blocks);
                    uint8_t jDev=0;
                    char * nextStrNewBuf = NULL;
                    for(uint8_t i=0;i<blockSize; i++){
                        cJSON * iBlock = cJSON_GetArrayItem(blocks,i);
                        uint8_t nDev = (uint8_t)cJSON_GetNumberValue(
                                    cJSON_GetObjectItem(iBlock, "n"));
                        nDev = nDev + jDev;
                        string  name = {0};
                        strcpy(name, cJSON_GetStringValue(
                                   cJSON_GetObjectItem(iBlock, "name")));
                        //...5.1 создаём прибор с именем name
                        uint8_t dbI = getDbIndex(&eqDB, curDevName);
                        cJSON * actualDev = getJsonByName(eqDB.devJson[dbI], name);
                        cJSON * devParams = actualDev->child;
                        char * testPStr = *pStr;
                        for(uint16_t i=0; devParams !=NULL &&
                                          i<MAX_SC_PARAMS; i++){
                            pStr = checkNames(devParams->string,
                                              devBuf, devBufSize);
                            testPStr = *pStr;
                            if(pStr){
                                //cJSON * curJson = getJsonByName(scJson, curParamName);
                                char * jStr = jsonPrint(devParams);
                                setStrParam(pStr, jStr);
                            }
                            devParams = devParams->next;
                        }
                        //...5.2 копируем  nDev с именем name
                        //...5.2.1 пропускаем строчку с общим числом БА
                        nextStrNewBuf = strstr(newBuf, "\n");
                        nextStrNewBuf++;
                        cJSON * orientation = getJsonByName(NumberOfDevice, "orientation");
                        for(;jDev<nDev; jDev++){
                            char * strAxis = *(checkNames("Axis", devBuf, devBufSize));
                            setDevicePosition(strAxis, &orientation[jDev]);
                            nextStrNewBuf = devicePrint(nextStrNewBuf, curDevName, jDev,
                                                        devBuf, devBufSize);
                        }
                        if(blockSize == 0){
                            nextStrNewBuf = devicePrint(nextStrNewBuf, curDevName, jDev,
                                                        devBuf, devBufSize);
                        }
                        //дочитываем до секции следующего прибора, если надо
                        while (!strstr(newDevSection, "*****")){
                            if(fgets(newDevSection, STR_BUF_SIZE, temp) == NULL){
                                break;
                            }
                        }
                        sprintf(nextStrNewBuf, "%s", newDevSection);
                    }
                }
                else{//4.1.2 простая строка - копируем как есть
                    cJSON * curJson = getJsonByName(scJson, curParamName);
                    char * jStr = jsonPrint(curJson);
                    setStrParam(newBuf, jStr);
                }
            }

            fprintf(out, "%s", curBuf);
            strcpy(curBuf, newBuf);
       }
       fclose(temp);
       fclose(out);
    }

    //cJSON * wheels = cJSON_GetObjectItem(scJson, "wheels");
    //uint32_t wheelSize = cJSON_GetArraySize(wheels);
}

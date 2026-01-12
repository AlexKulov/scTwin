#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../sctwin.h"

typedef struct StrConfig{
    string tName;
    string dName;
    uint8_t n;
}StrConfig;

typedef struct SysConfig{
    StrConfig str[MAX_SC_PARAMS];
    uint8_t size;
}SysConfig;

typedef struct DescDevType{
    string tName;
    string dName[MAX_DEV_OF_TYPE];
    uint8_t n;
}DescDevType;

typedef struct Syslib{
    DescDevType str[MAX_SC_PARAMS];
    uint8_t size;
}Syslib;

static SysConfig * genSysConfig;
static void printConfig(SysConfig * c){
    StrConfig str;
    for(int i = 0; i < MAX_SC_PARAMS; i++){
        str = c->str[i];
        if(str.n>0){
            printf("tName=%s, dName=%s, n=%i\n",
                   str.tName, str.dName, str.n);
        }
        else
            break;
    }
}

static void genSysConfigs(SysConfig * nowSysConfig, Syslib * myLib){
    uint32_t totalConfigs = 1;//возможное число конфигураций
    uint8_t deviceCount = 0; //количество строк устройств в конфигурации
    DescDevType * libStr[MAX_SC_PARAMS] = {NULL};//кол-во алтернатив для строки

    for(int i = 0; i < MAX_SC_PARAMS; i++) {
        if(!(nowSysConfig->str[i].n>0)){
            deviceCount=i;
            break;
        }
        // Ищем в библиотеке
        for(int j = 0; j < MAX_SC_PARAMS; j++) {
            if(myLib->str[j].n > 0 &&
               strcmp(nowSysConfig->str[i].tName, myLib->str[j].tName) == 0) {
                libStr[i] = &myLib->str[j];
                totalConfigs *= libStr[i]->n;
                break;
            }
        }
    }

    // Выделяем память
    genSysConfig = (SysConfig *)calloc(totalConfigs, sizeof(SysConfig));

    // Генерируем все комбинации
    for(uint32_t configIdx = 0; configIdx < totalConfigs; configIdx++) {
        uint32_t remainder = configIdx;

        for(int devIdx = 0; devIdx < deviceCount; devIdx++) {
            // Копируем базовые данные
            genSysConfig[configIdx].str[devIdx] = nowSysConfig->str[devIdx];

            // Если есть альтернативы из библиотеки, выбираем одну
            if(libStr[devIdx]) {
                uint32_t altIdx = remainder % libStr[devIdx]->n;
                strcpy(genSysConfig[configIdx].str[devIdx].dName,
                       libStr[devIdx]->dName[altIdx]);
                remainder /= libStr[devIdx]->n;
            }
        }
    }

    for(uint32_t i = 0; i < totalConfigs; i++){
        printf("******** genSysConfig[%i] *********\n", i);
        printConfig(&genSysConfig[i]);
    }
}

static void createSysLib(char * systemName, Syslib * lib){
    char curDevName[STR_SIZE] = {0};
    uint8_t size = 0;
    uint8_t num = 0;
    for(uint16_t i=0; i<MAX_SC_PARAMS; i++){
        strcpy(curDevName, devNameList[i]);
        uint16_t len = strlen(curDevName);
        if(!len)
            break;
        string devFileName = {0};
        sprintf(devFileName, "./%s/dev/%s.json", systemName, curDevName);
        char * scJsonFile = fileRead(devFileName);
        if(scJsonFile){
            cJSON * devJson = cJSON_Parse(scJsonFile);
            if(devJson){
                num = 0;
                strcpy(lib->str[size].tName, curDevName);
                cJSON * curDevJson = devJson->child;
                for(uint8_t i=0; curDevJson!=NULL &&
                                 i<MAX_DEV_OF_TYPE; i++){
                    strcpy(lib->str[size].dName[num], curDevJson->string);
                    num++;
                    curDevJson = curDevJson->next;
                }
                lib->str[size].n = num;
                size++;
            }
            else
                printf("createSysLib: Can't json parse  %s file\n", devFileName);
        }
        else
            printf("createSysLib: Can't open  %s file\n", devFileName);
    }
    lib->size = size;
}

static void createSysConfig(char * sysName, char * confName, SysConfig * conf){
    string sysConfFile = {0};
    sprintf(sysConfFile, "./%s/%s.json", sysName, confName);
    char * sysConf = fileRead(sysConfFile);
    if(sysConf){
        cJSON * sysConfJson = cJSON_Parse(sysConf);
        if(sysConfJson){
            uint8_t nStr = 0;
            for(uint8_t i=0; i<MAX_DEV_OF_TYPE; i++){
                cJSON * nDev = NULL;
                cJSON * nameDev = NULL;
                cJSON * device = getJsonByName(sysConfJson, devNameList[i]);
                if(device){
                    cJSON * blocs = getJsonByName(device, "blocks");
                    nDev    = cJSON_GetObjectItem(cJSON_GetArrayItem(blocs, 0), "n");
                    nameDev = cJSON_GetObjectItem(cJSON_GetArrayItem(blocs, 0), "name");
                    uint8_t n = cJSON_GetNumberValue(nDev);
                    char * name = cJSON_GetStringValue(nameDev);
                    strcpy(conf->str[nStr].dName, name);
                    strcpy(conf->str[nStr].tName, devNameList[i]);
                    conf->str[nStr].n = n;
                    nStr++;
                }
            }
            conf->size = nStr;
        }
        else
            printf("createSysConfig: Can't json parse  %s file\n", sysConfFile);
    }
    else
        printf("createSysConfig: Can't open  %s file\n", sysConfFile);
}

static void setXyzJson(cJSON * subItem, double xyz[3]){
    cJSON *
    xyzJson = cJSON_CreateNumber(xyz[0]);
    cJSON_AddItemToObject(subItem, "x", xyzJson);
    xyzJson = cJSON_CreateNumber(xyz[1]);
    cJSON_AddItemToObject(subItem, "y", xyzJson);
    xyzJson = cJSON_CreateNumber(xyz[2]);
    cJSON_AddItemToObject(subItem, "z", xyzJson);
}

static void createSysJson(SysConfig * conf, char * sysName, char * tempConfName){
    string tempSysConfFile = {0};
    sprintf(tempSysConfFile, "./%s/%s.json", sysName, tempConfName);
    char * tempSysConf = fileRead(tempSysConfFile);
    if(tempSysConf){
        cJSON * tempJson = cJSON_Parse(tempSysConf);
        if(tempJson){
            char * resConvert;
            for(uint8_t i=0; i<conf->size; i++){
                cJSON * devType = getJsonByName(tempJson, conf->str[i].tName);
                cJSON * blocks = NULL;
                /* просто заменяем имя прибора в блоке */
                if(devType){
                    blocks = cJSON_GetObjectItem(devType, "blocks");
                    cJSON * name = cJSON_GetObjectItem(
                                   cJSON_GetArrayItem(blocks, 0), "name");
                    char * dNameSet =
                            cJSON_SetValuestring(name, conf->str[i].dName);
                }
                /* если прибора нет, то создаётся блок с этим устройством */
                else{
                    devType = cJSON_CreateObject();
                    //blocks
                    cJSON * blocks       = cJSON_AddArrayToObject(devType, "blocks");
                    cJSON * orientation = cJSON_AddArrayToObject(devType, "orientation");
                    cJSON * item        = cJSON_CreateObject();
                    //{"name": "Panel1","n": 3}
                    cJSON * subItem      = cJSON_CreateString(conf->str[i].dName);
                    cJSON_AddItemToObject(item, "name", subItem);
                            subItem      = cJSON_CreateNumber(conf->str[i].n);
                    cJSON_AddItemToObject(item, "n", subItem);
                    cJSON_AddItemToArray(blocks, item);
                    for(uint8_t j=0; j<conf->str[i].n; j++){
                        subItem = cJSON_CreateString("Z");
                        item        = cJSON_CreateObject();
                        cJSON_AddItemToObject(item, "mainAxis", subItem);

                        double xyz[3] = {1, 0, 0};
                        subItem = cJSON_CreateObject();
                        setXyzJson(subItem, xyz);

                        cJSON_AddItemToObject(item, "mainAxisInBody", subItem);
                        xyz[0] = 0; xyz[1] = 0; xyz[2] = -1;
                        subItem = cJSON_CreateObject();
                        setXyzJson(subItem, xyz);

                        cJSON_AddItemToObject(item, "secnAxisInBody", subItem);
                        cJSON_AddItemToArray(orientation, item);
                    }
                    //char * devTypePrint = cJSON_Print(devType);
                    //printf("\n%s\n", devTypePrint);
                    string  devTypeName = {0};
                    sprintf(devTypeName, "Number of %s", conf->str[i].tName);
                    cJSON_AddItemToObject(tempJson, devTypeName, devType);
                }
                /***********************************************************/
            }

            cJSON * numberOf = tempJson->child;
            long isTypeNameExist = 0;
            /* удаляем блоки, если таких устройств нет в конфигурации */
            for(uint8_t i=0; numberOf; i++){
                if(strstr(numberOf->string, "Number of ")){
                    isTypeNameExist = 0;
                    for(uint8_t j=0; j<conf->size; j++){
                        if(strstr(numberOf->string, conf->str[j].tName)){
                            isTypeNameExist = 1;
                            break;
                        }
                    }

                    if(!isTypeNameExist){ //если не существует - выкидываем
                        cJSON * delDev = numberOf;
                        numberOf = numberOf->next;
                        cJSON_DetachItemViaPointer(tempJson, delDev);
                        cJSON_Delete(delDev);
                    }
                    else
                        numberOf = numberOf->next;
                }
                else
                    numberOf = numberOf->next;
            }
            /***********************************************************/
            resConvert = cJSON_Print(tempJson);
            printf("\n%s\n", resConvert);
        }
        else
            printf("createSysJson: Can't json parse  %s file\n", tempSysConfFile);
    }
    else
        printf("createSysJson: Can't open  %s file\n", tempSysConfFile);
}

void testGenConfigs(){

    char * systemName = "acos";
    Syslib sysLib;
    createSysLib(systemName, &sysLib);
    //SysConfig nowSysConfig;
    char * confName = "sc2";
    //createSysConfig(systemName, confName, &nowSysConfig);
    //printConfig(&nowSysConfig);
       SysConfig nowSysConfig = {
            .str[0].tName = "wheel", .str[0].dName = "UDM1", .str[0].n = 4,
            .str[1].tName = "Star" , .str[1].dName = "ST1" , .str[1].n = 2,
            .str[2].tName = "Gyro" , .str[2].dName = "IMU1", .str[2].n = 2,
            .size = 3
        };

       createSysJson(&nowSysConfig, systemName, confName);

    /*Syslib myLib = {
        .str[0].tName = "wheel", .str[0].dName[0] = "UDM1", .str[0].dName[1] = "UDM2",
                                 .str[0].dName[2] = "UDM3", .str[0].dName[3] = "UDM4",
                                 .str[0].dName[4] = "UDM5", .str[0].n = 5,
        .str[1].tName = "Star" , .str[1].dName[0] = "ST1" , .str[1].dName[1] = "ST2" , .str[1].n = 2,
        .str[2].tName = "MTB"  , .str[2].dName[0] = "EM1" ,                            .str[2].n = 1,
        .str[3].tName = "Gyro" , .str[3].dName[0] = "IMU1", .str[3].dName[1] = "BCHE", .str[3].n = 2
    };*/

    for(int i = 0; i < sysLib.size; i++){
        printf("******** LibTypeConfig[%i] *********\n", i);
        if(sysLib.str[i].n==1)
            printf("tName=%s, dName1=%s, n=1\n",
                   sysLib.str[i].tName, sysLib.str[i].dName[0]);
        else if(sysLib.str[i].n==2)
            printf("tName=%s, dName1=%s, dName2=%s, n=2\n",
                   sysLib.str[i].tName, sysLib.str[i].dName[0], sysLib.str[i].dName[1]);
        else if(sysLib.str[i].n>2)
            printf("tName=%s, dName1=%s, ... n=%i\n",
                   sysLib.str[i].tName, sysLib.str[i].dName[0], sysLib.str[i].n);
    }
    genSysConfigs(&nowSysConfig, &sysLib);
}

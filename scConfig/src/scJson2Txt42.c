#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cJSON.h"

#include "../sctwin.h"

#define MAX_SC_PARAMS (20)
#define STR_BUF_SIZE 512
static string scParams[MAX_SC_PARAMS];
const
static string eqList[MAX_SC_PARAMS] = {
    "wheel"
    "MTB",
    "Thr",
    "Gyro",
    "Mag", //Magnetometer
    "Coarse",//Coarse Sun Sensor
    "Fine Sun Sensor",//Fine Sun Sensor
    "Star",//Star Tracker
    "Accel", //Accelerometer
    "Fine Guidance Sensor" //Fine Guidance Sensor
};

typedef struct EqDescription{
    string name;
    cJSON * json;
    FILE * file;
}EqDescription;

typedef struct EqDataBase{
    EqDescription description[MAX_SC_PARAMS];
    uint8_t size;
}EqDataBase;
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

static cJSON * findScParams(cJSON * scJson, char * str){
    for(uint32_t i=0;i<MAX_SC_PARAMS;i++){
        if(strstr(str,scParams[i])){
            //-------------- Это актуатор ? ----------------
            for(uint8_t j=0; j<N_ACT; j++){
                if(strstr(str, actNames[j])){
                    actFlag[j]++;
                    uint8_t summa = actFlag[0]+
                         actFlag[1]+actFlag[2];
                    if(actFlag[j] == 2 && (summa%2) == 0){
                        return ACTUATOR;
                    }
                    else if(actFlag[j] == 2 && (summa%2) == 1){
                        printf("Warning: Structure of SC-file does not look like 42inp-file!\n");
                        return ERROR;
                    }
                    return NOTHING;
                }
            }
            //-------------- Это сенсор ? ----------------
            for(uint8_t j=0; j<N_SNS; j++){
                if(strstr(str, snsNames[j])){
                    return SENSOR;
                }
            }
            //-------- Это просто параметр ! -------------
            return SC_PARAM;
        }
    }
    return NOTHING;
}

static string * checkNames(char * str, string * arNames, uint8_t nAr){
    for(uint8_t i=0; i<nAr; i++){
        if(strstr(str,arNames[i])){
            return 1;
        }
    }
    return 0;
}

void scJson2Txt42(cJSON * scJson, char * tempName, char * outName){
    FILE * temp = fopen(tempName, "r");
    FILE * out  = fopen(outName,  "w");
    if(temp && out && scJson){
        //1. Открываем файл шаблона и начинаем его переписывать в выход.
        cJSON * curParam = scJson->child;
        uint16_t nParams = 0;
        uint16_t nEqList = 0;
        string * curStr = NULL;
        FILE * eqFiles[MAX_SC_PARAMS] = {0};
        //2. считаем сколько в параметрах строк аппаратуры
        for(uint16_t i=0; curParam !=NULL &&
                          nParams<MAX_SC_PARAMS; i++){
            strcpy(scParams[i],curParam->string);
            curStr = checkNames(scParams[i], eqList, MAX_SC_PARAMS);
            if(curStr){
                strcpy(eqCurList[nEqList], curStr);
                string eqFileName = {0};
                sprintf(eqFileName, "./acos/dev/%s.json", curStr);
                eqFiles[nEqList] = fopen(eqFileName, "r");
                nEqList++;
            }
            curParam = curParam->next;
            nParams++;
        }

        char curBuf[1024]={0};
        char newBuf[1024]={0};
        //int endOfFile =
        fscanf(temp, "%1023[^\n]\n", curBuf);
        //3. запускаем построчное чтение
        while (fscanf(temp, "%1023[^\n]\n", newBuf) != EOF) {
            //4. Проверка, если строка из файла содержит любой параметр
            curStr = checkNames(newBuf, scParams, nParams);
            if(curStr){
                //4.1 Проверка, если параметр содержит строку аппаратуры
                curStr = checkNames(curStr, eqCurList, nEqList);
                if(curStr){
                    //...1 считаем кол-во аппаратуры данного типа
                    //uint8_t nEq = getNumEquipment(cJSON * scJson, curStr);
                    //...2 модифицируем строку с кол-вом аппаратуры
                    //...3 считать строку с номером 0 прибора в буфер 0
                    //...4 считать строки типового описания в буфер1
                    //...4 если после считывания не начался новый прибор, то дойти до нового
                    //...5 цикл по типам приборов
                    //...5.1 заполняем буфер1 описание текущего типа
                    //...5.2 цикл по кол-ву приборов данного типа
                    //...5.2.1 модифицируем буфер0 добавляя номер текущего прибора
                    //...5.2.2 модифицируем буфер1 добавляя ориентацию текущего прибора
                    //...5.2.3 записываем буфер0+1 с описанием в выходной файл
                }
                else{//4.1.2 простая строка - копируем как есть
                    char param[256], desc[256];
                    sscanf(newBuf, "%s|%s", param, desc);

                    //cJSON * jsonParam = cJSON_GetObjectItem(scJson, );
                    double somethig = 0;
                    sprintf(param,"%s ", somethig);
                    sprintf(newBuf, "%s|%s", param, desc);
                }
            }
       }
       fclose(temp);
       fclose(out);
    }

    //cJSON * wheels = cJSON_GetObjectItem(scJson, "wheels");
    //uint32_t wheelSize = cJSON_GetArraySize(wheels);
}

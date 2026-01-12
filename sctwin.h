#ifndef SCTWIN_H
#define SCTWIN_H

#include "cJSON.h"

#ifndef FALSE
    #define FALSE (0)
    #define TRUE (1)
#endif

#define STR_SIZE (254)
#define DESC_STR_MAX_42 (64)
#define MAX_SC_PARAMS (20)
#define STR_BUF_SIZE (2048)
#define MAX_DEV_OF_TYPE (8)

typedef char string [STR_SIZE];


extern char* fileRead(const char *filename);

extern const string devNameList[MAX_SC_PARAMS];
extern  cJSON * getJsonByName(const cJSON * json, const string name);
extern  string * checkNames(const char * str, string * arNames, uint8_t nAr);
extern void scJson2Txt42(cJSON * scJson, char * tempName,
                         char * outName, char * sysName);

#endif // SCTWIN_H

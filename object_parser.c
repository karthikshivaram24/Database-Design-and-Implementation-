//
// Created by Saurabh Tiwari on 15/11/16.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "object_parser.h"

int split (char *str, char c, char ***arr)
{
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
}

typedef struct VarString {
    char *buf;
    int size;
    int bufsize;
} VarString;


Schema *stringToSchemaParser(char *data)
{
    Schema * schema = (Schema *) malloc(sizeof(Schema));
    int c = 0, i;
    char **arr = NULL;
    char **attributeArray = NULL;

    c = split(data, '<', &arr);
    c = split(arr[1], '>', &attributeArray);

    int attrNum = atoi(attributeArray[0]);
    schema->numAttr = attrNum;
    schema->attrNames = (char **) malloc(sizeof(char*)*attrNum);
    schema->dataTypes = (DataType *) malloc(sizeof(DataType)*attrNum);
    schema->typeLength = (int *) malloc(sizeof(int)*attrNum);



    char *attributeStringPointer = strstr(attributeArray[1], "(")+1;
    char * substring = strstr(attributeStringPointer, ")");
    int length = substring - attributeStringPointer;
    char *attributeString = malloc(length);
    memcpy(attributeString, attributeStringPointer, length);
    char **datatypeArray = NULL;
    c = split(attributeString, ',', &datatypeArray);
    char * splitAttribute = (char *) malloc(sizeof(char));
    for (i = 0; i < c; i++)
    {
        splitAttribute = datatypeArray[i];

        splitAttribute = strtok(splitAttribute, ": ");
        schema->attrNames[i] = (char *)malloc(sizeof(char));
        schema->attrNames[i] = splitAttribute;
        splitAttribute = strtok(NULL, ": ");
        if(strcmp(splitAttribute,"INT")==0)
        {
            schema->dataTypes[i] = DT_INT;
            schema->typeLength[i] = 0;
        }
        else if(strcmp(splitAttribute,"FLOAT")==0)
        {
            schema->dataTypes[i] = DT_FLOAT;
            schema->typeLength[i] = 0;
        }
        else if(strcmp(splitAttribute,"BOOL")==0)
        {
            schema->dataTypes[i] = DT_BOOL;
            schema->typeLength[i] = 0;
        }
        else
        {
            splitAttribute = strtok(splitAttribute,"[");
            splitAttribute = strtok(NULL,"[");
            splitAttribute = strtok(splitAttribute,"]");
            int strLength = atoi(splitAttribute);
            schema->dataTypes[i]= DT_STRING;
            schema->typeLength[i] = strLength;

        }
    }
    char * keyAttribute = (char *) malloc(sizeof(char));
    keyAttribute = strtok(substring,":");
    keyAttribute = strtok(NULL, ":");
    keyAttribute =strtok(keyAttribute,"(");
    keyAttribute =strtok(NULL,"(");

    keyAttribute =strtok(keyAttribute,")");
    keyAttribute = strtok(keyAttribute, ", ");

    int keyNum = 0;
    char **keys = (char**) malloc(sizeof(char*)*schema->numAttr);
    while (keyAttribute)
    {
        keys[keyNum] = (char*)malloc(sizeof(char*));
        strcpy(keys[keyNum], keyAttribute);
        keyNum++;

        keyAttribute = strtok(NULL, ", ");
    }
    free(keyAttribute);
    schema->keyAttrs = (int *)malloc(sizeof(int)* keyNum);
    schema->keySize = keyNum;

    int j;
    for (i = 0; i<keyNum; i++)
    {
        for (j = 0; j<schema->numAttr; j++)
        {
            if(strcmp(keys[i], schema->attrNames[j])==0)
            {
                schema->keyAttrs[i] = j;
                break;
            }
        }
    }
    free(arr);
    free(attributeArray);
    free(datatypeArray);

    arr = NULL;
    attributeArray = NULL;
    datatypeArray = NULL;

    return schema;

}




Record *stringToRecordParser(char *data, Schema *schema)
{
    int i, lastAttr = schema->numAttr-1;
    int intVal;
    float floatVal;
    bool boolVal;


    Value *value;
    Record *record = (Record*)malloc(sizeof(Record*));
    record->data = (char*)malloc(sizeof(char*));

    char *splitStart, *splitEnd;

    splitStart = strtok(data,"(");

    for(i=0;i< schema->numAttr;i++)
    {
        splitEnd = strtok(NULL,":");

        if(i == lastAttr)
        {
            splitEnd = strtok(NULL,")");
        }
        else
        {
            splitEnd = strtok(NULL,",");
        }

        switch(schema->dataTypes[i])
        {
            case DT_INT:
                intVal = strtol(splitEnd, &splitStart, 10);
                MAKE_VALUE(value,DT_INT,intVal);
                setAttr(record,schema,i,value);
                free(value);
                break;

            case DT_FLOAT:
                floatVal = strtof(splitEnd, NULL);
                MAKE_VALUE(value,DT_FLOAT,floatVal);
                setAttr(record,schema,i,value);
                free(value);
                break;

            case DT_BOOL:
                boolVal = (splitEnd[0] == 't') ? TRUE: FALSE;
                MAKE_VALUE(value,DT_BOOL,boolVal);
                setAttr(record,schema,i,value);
                free(value);
                break;

            case DT_STRING:
                MAKE_STRING_VALUE(value,splitEnd);
                setAttr (record,schema,i,value);
                freeVal(value);
                break;
        }

    }

    return record;
}

/**
 * Finds the record size and also finds the offset of a given file as well.
 * @param schema
 * @param attrNum
 * @return
 */
int
getRecordSizeOffset(Schema *schema, int attrNum)
{
    int offset = 0;
    int attrPos = 0;

    for(attrPos = 0; attrPos < attrNum; attrPos++)
        switch (schema->dataTypes[attrPos])
        {
            case DT_STRING:
                offset += schema->typeLength[attrPos];
                break;
            case DT_INT:
                offset += sizeof(int);
                break;
            case DT_FLOAT:
                offset += sizeof(float);
                break;
            case DT_BOOL:
                offset += sizeof(bool);
                break;
        }
    return offset;
}

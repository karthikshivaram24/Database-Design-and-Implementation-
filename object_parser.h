//
// Created by Saurabh Tiwari on 16/11/16.
//

#ifndef OBJECT_PARSER_H_H
#define OBJECT_PARSER_H_H

#include "tables.h"
#include "record_mgr.h"

extern int split (char *str, char c, char ***arr);
extern Schema *stringToSchemaParser(char *data);
extern Record *stringToRecordParser(char *data, Schema *schema);
extern int getRecordSizeOffset(Schema *schema, int attrNum);

#endif //ASG2_GROUP25_OBJECT_PARSER_H_H

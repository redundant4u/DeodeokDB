#ifndef table_h
#define table_h

#include <stdlib.h>
#include <unistd.h>

#include "pager.h"

typedef struct {
    Pager *pager;
    uint32_t numRows;
} Table;

#define TABLE_MAX_PAGES 100

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define sizeOfAttribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

Table *dbOpen(const char *filename);
void dbClose(Table *table);

#endif

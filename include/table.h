#ifndef table_h
#define table_h

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "pager.h"

typedef struct {
    Pager *pager;
    uint32_t rootPageNum;
} Table;

typedef struct {
    Table *table;
    uint32_t pageNum;
    uint32_t cellNum;
    bool endOfTable;
} Cursor;

#define TABLE_MAX_PAGES 100

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define sizeOfAttribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

Table *dbOpen(const char *filename);
void dbClose(Table *table);

Cursor *tableStart(Table *table);
Cursor *tableFind(Table *table, uint32_t key);

void *cursorValue(Cursor *cursor);
void cursorAdvance(Cursor *cursor);

#endif

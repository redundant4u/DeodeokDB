#ifndef table_h
#define table_h

#include "common.h"

#define TABLE_MAX_PAGES 100

typedef struct {
    uint32_t numRows;
    void *pages[TABLE_MAX_PAGES];
} Table;

Table *newTable();
void freeTable(Table *table);

#endif

#ifndef cursor_h
#define cursor_h

#include <stdbool.h>
#include <unistd.h>

#include "table.h"

typedef struct {
    Table *table;
    uint32_t pageNum;
    uint32_t cellNum;
    bool endOfTable;
} Cursor;

Cursor *tableStart(Table *table);
Cursor *tableEnd(Table *table);
void *cursorValue(Cursor *cursor);
void cursorAdvance(Cursor *cursor);

#endif

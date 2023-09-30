#include "cursor.h"

Cursor *tableStart(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));

    cursor->table = table;
    cursor->rowNum = 0;
    cursor->endOfTable = (table->numRows == 0);

    return cursor;
}

Cursor *tableEnd(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));

    cursor->table = table;
    cursor->rowNum = table->numRows;
    cursor->endOfTable = true;

    return cursor;
}

void cursorAdvance(Cursor *cursor) {
    cursor->rowNum += 1;

    if (cursor->rowNum >= cursor->table->numRows) {
        cursor->endOfTable = true;
    }
}

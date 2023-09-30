#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pager.h"
#include "row.h"

void serializeRow(Row *source, void *destination) {
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    strncpy(destination + USERNAME_OFFSET, source->username, USERNAME_SIZE);
    strncpy(destination + EMAIL_OFFSET, source->email, EMAIL_SIZE);
}

void deserializeRow(void *source, Row *destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

void *cursorValue(Cursor *cursor) {
    uint32_t rowNum = cursor->rowNum;
    uint32_t pageNum = rowNum / ROWS_PER_PAGE;

    void *page = getPage(cursor->table->pager, pageNum);

    uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * ROW_SIZE;

    return page + byteOffset;
}

void printRow(Row *row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

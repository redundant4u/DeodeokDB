#include "cursor.h"
#include "tree.h"

Cursor *tableStart(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));

    cursor->table = table;
    cursor->pageNum = table->rootPageNum;
    cursor->cellNum = 0;

    void *rootNode = getPage(table->pager, table->rootPageNum);
    uint32_t numCells = *leafNodeNumCells(rootNode);
    cursor->endOfTable = (numCells == 0);

    return cursor;
}

Cursor *tableEnd(Table *table) {
    Cursor *cursor = malloc(sizeof(Cursor));

    cursor->table = table;
    cursor->pageNum = table->rootPageNum;

    void *rootNode = getPage(table->pager, table->rootPageNum);
    uint32_t numCells = *leafNodeNumCells(rootNode);
    cursor->cellNum = numCells;
    cursor->endOfTable = true;

    return cursor;
}

void *cursorValue(Cursor *cursor) {
    uint32_t pageNum = cursor->pageNum;

    void *page = getPage(cursor->table->pager, pageNum);

    return leafNodeValue(page, cursor->cellNum);
}

void cursorAdvance(Cursor *cursor) {
    uint32_t pageNum = cursor->pageNum;
    void *node = getPage(cursor->table->pager, pageNum);

    cursor->cellNum += 1;
    if (cursor->cellNum >= (*leafNodeNumCells(node))) {
        cursor->endOfTable = true;
    }
}

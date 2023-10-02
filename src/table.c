#include <stdio.h>
#include <string.h>

#include "row.h"
#include "table.h"
#include "tree.h"

Table *dbOpen(const char *filename) {
    Pager *pager = pagerOpen(filename);

    Table *table = (Table *)malloc(sizeof(Table));
    table->pager = pager;
    table->rootPageNum = 0;

    if (pager->numPages == 0) {
        void *rootNode = getPage(pager, 0);
        initializeLeafNode(rootNode);
    }

    return table;
}

void dbClose(Table *table) {
    Pager *pager = table->pager;

    for (uint32_t i = 0; i < pager->numPages; i++) {
        if (pager->pages[i] == NULL) {
            continue;
        }

        pagerFlush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int result = close(pager->fileDescriptor);
    if (result == -1) {
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void *page = pager->pages[i];

        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }

    free(pager);
    free(table);
}

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
Cursor *tableFind(Table *table, uint32_t key) {
    uint32_t rootPageNum = table->rootPageNum;
    void *rootNode = getPage(table->pager, rootPageNum);

    if (getNodeType(rootNode) == NODE_LEAF) {
        return leafNodeFind(table, rootPageNum, key);
    } else {
        printf("Need to implement searching an internal node\n");
        exit(EXIT_FAILURE);
    }
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

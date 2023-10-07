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
        setNodeRoot(rootNode, true);
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
    Cursor *cursor = tableFind(table, 0);

    void *node = getPage(table->pager, cursor->pageNum);
    uint32_t numCells = *leafNodeNumCells(node);
    cursor->endOfTable = (numCells == 0);

    return cursor;
}

Cursor *tableFind(Table *table, uint32_t key) {
    uint32_t rootPageNum = table->rootPageNum;
    void *rootNode = getPage(table->pager, rootPageNum);

    if (getNodeType(rootNode) == NODE_LEAF) {
        return leafNodeFind(table, rootPageNum, key);
    } else {
        return internalNodeFind(table, rootPageNum, key);
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
        // Advance to next leaf node
        uint32_t nextPageNum = *leafNodeNextLeaf(node);

        if (nextPageNum == 0) {
            cursor->endOfTable = true;
        } else {
            cursor->pageNum = nextPageNum;
            cursor->cellNum = 0;
        }
    }
}

#include <stdio.h>
#include <string.h>

#include "row.h"
#include "table.h"

Table *dbOpen(const char *filename) {
    Pager *pager = pagerOpen(filename);
    uint32_t numRows = pager->fileLength / ROW_SIZE;

    Table *table = (Table *)malloc(sizeof(Table));
    table->pager = pager;
    table->numRows = numRows;

    return table;
}

void dbClose(Table *table) {
    Pager *pager = table->pager;
    uint32_t numFullPages = table->numRows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < numFullPages; i++) {
        if (pager->pages[i] == NULL) {
            continue;
        }

        pagerFlush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    uint32_t numAdditionalRows = table->numRows % ROWS_PER_PAGE;
    if (numAdditionalRows > 0) {
        uint32_t pageNum = numFullPages;

        if (pager->pages[pageNum] != NULL) {
            pagerFlush(pager, pageNum, numAdditionalRows * ROW_SIZE);
            free(pager->pages[pageNum]);
            pager->pages[pageNum] = NULL;
        }
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

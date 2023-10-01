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

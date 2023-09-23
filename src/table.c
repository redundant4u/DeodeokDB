#include <stdlib.h>

#include "table.h"

Table *newTable() {
    Table *table = (Table *)malloc(sizeof(Table));
    table->numRows = 0;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }

    return table;
}

void freeTable(Table *table) {
    for (int i = 0; table->pages[i]; i++) {
        free(table->pages[i]);
    }
    free(table);
}

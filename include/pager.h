#ifndef pager_h
#define pager_h

#define TABLE_MAX_PAGES 400

typedef struct {
    int fileDescriptor;
    uint32_t fileLength;
    uint32_t numPages;
    void *pages[TABLE_MAX_PAGES];
} Pager;

#include <unistd.h>

#include "table.h"

Pager *pagerOpen(const char *filename);
void *getPage(Pager *pager, uint32_t pageNum);
void pagerFlush(Pager *pager, uint32_t pageNum);

#endif

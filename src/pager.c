#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "pager.h"
#include "row.h"

Pager *pagerOpen(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT | S_IWUSR | S_IRUSR);

    if (fd == -1) {
        printf("Unable to open file.\n");
        exit(EXIT_FAILURE);
    }

    off_t fileLength = lseek(fd, 0, SEEK_END);

    Pager *pager = malloc(sizeof(Pager));
    pager->fileDescriptor = fd;
    pager->fileLength = fileLength;
    pager->numPages = fileLength / PAGE_SIZE;

    if (fileLength % PAGE_SIZE != 0) {
        printf("DB file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

void *getPage(Pager *pager, uint32_t pageNum) {
    if (pageNum > TABLE_MAX_PAGES) {
        printf("Tried to fetch page number out of bounds. %d > %d\n", pageNum,
               TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    // Cache miss. Allocate memory and load from file.
    if (pager->pages[pageNum] == NULL) {
        void *page = malloc(PAGE_SIZE);
        uint32_t numPages = pager->fileLength / PAGE_SIZE;

        if (pager->fileLength % PAGE_SIZE) {
            numPages += 1;
        }

        if (pageNum <= numPages) {
            lseek(pager->fileDescriptor, pageNum * PAGE_SIZE, SEEK_SET);
            ssize_t bytesRead = read(pager->fileDescriptor, page, PAGE_SIZE);

            if (bytesRead == -1) {
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }

        pager->pages[pageNum] = page;

        if (pageNum >= pager->numPages) {
            pager->numPages = pageNum + 1;
        }
    }

    return pager->pages[pageNum];
}

void pagerFlush(Pager *pager, uint32_t pageNum) {
    if (pager->pages[pageNum] == NULL) {
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->fileDescriptor, pageNum * PAGE_SIZE, SEEK_SET);

    if (offset == -1) {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bytesWritten =
        write(pager->fileDescriptor, pager->pages[pageNum], PAGE_SIZE);

    if (bytesWritten == -1) {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

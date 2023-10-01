#ifndef row_h
#define row_h

#include <unistd.h>

#include "table.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

#define sizeOfAttribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

static const uint32_t ID_SIZE = sizeOfAttribute(Row, id);
static const uint32_t USERNAME_SIZE = sizeOfAttribute(Row, username);
static const uint32_t EMAIL_SIZE = sizeOfAttribute(Row, email);

static const uint32_t ID_OFFSET = 0;
static const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
static const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;

static const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

static const uint32_t PAGE_SIZE = 4096;

void serializeRow(Row *source, void *destination);
void deserializeRow(void *source, Row *destination);
void printRow(Row *row);

#endif

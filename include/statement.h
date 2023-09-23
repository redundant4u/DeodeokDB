#ifndef statement_h
#define statement_h

#include "row.h"
#include "table.h"

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef struct {
    StatementType type;
    Row rowToInsert;
} Statement;

PrepareResult prepareStatement(InputBuffer *inputBuffer, Statement *statement);
ExecuteResult executeInsert(Statement *statement, Table *table);
ExecuteResult executeSelect(Statement *statement, Table *table);
ExecuteResult executeStatement(Statement *statement, Table *table);

#endif

#ifndef statement_h
#define statement_h

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef struct {
    StatementType type;
} Statement;

PrepareResult prepareStatement(InputBuffer *inputBuffer, Statement *statement);
void executeStatement(Statement *statement);

#endif

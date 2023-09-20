#include <stdio.h>
#include <string.h>

#include "command.h"
#include "statement.h"

PrepareResult prepareStatement(InputBuffer *inputBuffer, Statement *statement) {
    if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }

    if (strcmp(inputBuffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void executeStatement(Statement *statement) {
    switch (statement->type) {
    case STATEMENT_INSERT:
        printf("This is where we would do an insert.\n");
        break;
    case STATEMENT_SELECT:
        printf("This is where we would do an select.\n");
        break;
    }
}

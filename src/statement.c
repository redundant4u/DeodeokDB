#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "statement.h"
#include "table.h"
#include "tree.h"

PrepareResult prepareInsert(InputBuffer *inputBuffer, Statement *statement) {
    statement->type = STATEMENT_INSERT;

    strtok(inputBuffer->buffer, " ");
    char *idString = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");

    if (idString == NULL || username == NULL || email == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }

    int id = atoi(idString);

    if (id < 0) {
        return PREPARE_NEGATIVE_ID;
    }

    if (strlen(username) > COLUMN_USERNAME_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->rowToInsert.id = id;
    strcpy(statement->rowToInsert.username, username);
    strcpy(statement->rowToInsert.email, email);

    return PREPARE_SUCCESS;
}

PrepareResult prepareStatement(InputBuffer *inputBuffer, Statement *statement) {
    if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
        return prepareInsert(inputBuffer, statement);
    }

    if (strcmp(inputBuffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult executeInsert(Statement *statement, Table *table) {
    void *node = getPage(table->pager, table->rootPageNum);

    uint32_t numCells = (*leafNodeNumCells(node));

    if (numCells >= LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }

    Row *rowToInsert = &(statement->rowToInsert);
    uint32_t keyToInsert = rowToInsert->id;
    Cursor *cursor = tableFind(table, keyToInsert);

    if (cursor->cellNum < numCells) {
        uint32_t keyAtIndex = *leafNodeKey(node, cursor->cellNum);

        if (keyAtIndex == keyToInsert) {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    leafNodeInsert(cursor, rowToInsert->id, rowToInsert);

    free(cursor);

    return EXECUTE_SUCCESS;
}

ExecuteResult executeSelect(Statement *statement, Table *table) {
    Cursor *cursor = tableStart(table);
    Row row;

    while (!(cursor->endOfTable)) {
        deserializeRow(cursorValue(cursor), &row);
        printRow(&row);
        cursorAdvance(cursor);
    }

    free(cursor);

    return EXECUTE_SUCCESS;
}

ExecuteResult executeStatement(Statement *statement, Table *table) {
    switch (statement->type) {
    case STATEMENT_INSERT:
        return executeInsert(statement, table);
    case STATEMENT_SELECT:
        return executeSelect(statement, table);
    }
}

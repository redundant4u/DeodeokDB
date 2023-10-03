#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "tree.h"

MetaCommandResult doMetaCommand(InputBuffer *inputBuffer, Table *table) {
    if (strcmp(inputBuffer->buffer, ".exit") == 0) {
        dbClose(table);
        exit(EXIT_SUCCESS);
    } else if (strcmp(inputBuffer->buffer, ".btree") == 0) {
        printf("Tree:\n");
        printTree(table->pager, 0, 0);
        return META_COMMAND_SUCCESS;
    } else if (strcmp(inputBuffer->buffer, ".constants") == 0) {
        printf("Constants:\n");
        printConstants();
        return META_COMMAND_SUCCESS;
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

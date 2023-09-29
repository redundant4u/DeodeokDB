#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "table.h"

MetaCommandResult doMetaCommand(InputBuffer *inputBuffer, Table *table) {
    if (strcmp(inputBuffer->buffer, ".exit") == 0) {
        dbClose(table);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

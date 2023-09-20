#include <stdlib.h>
#include <string.h>

#include "command.h"

MetaCommandResult doMetaCommand(InputBuffer *inputBuffer) {
    if (strcmp(inputBuffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

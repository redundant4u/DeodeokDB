#ifndef command_h
#define command_h

#include "buffer.h"

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum { PREPARE_SUCCESS, PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;

MetaCommandResult doMetaCommand(InputBuffer *inputBuffer);

#endif

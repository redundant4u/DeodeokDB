#include <stdlib.h>

#include "buffer.h"

InputBuffer *newInputBuffer()
{
    InputBuffer *inputBuffer = (InputBuffer *)malloc(sizeof(InputBuffer));

    inputBuffer->buffer = NULL;
    inputBuffer->bufferLength = 0;
    inputBuffer->inputLength = 0;

    return inputBuffer;
}

void closeInputBuffer(InputBuffer *inputBuffer)
{
    free(inputBuffer->buffer);
    free(inputBuffer);
}

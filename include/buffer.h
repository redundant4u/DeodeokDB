#ifndef buffer_h
#define buffer_h

#include <unistd.h>

typedef struct {
    char *buffer;
    size_t bufferLength;
    ssize_t inputLength;
} InputBuffer;

InputBuffer *newInputBuffer();
void closeInputBuffer(InputBuffer *inputBuffer);

#endif

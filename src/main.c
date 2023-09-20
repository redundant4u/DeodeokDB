#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

void printPrompt()
{
    printf("db > ");
}

void readInput(InputBuffer *inputBuffer)
{
    ssize_t bytesRead = getline(&(inputBuffer->buffer), &(inputBuffer->bufferLength), stdin);

    if (bytesRead <= 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    inputBuffer->inputLength = bytesRead - 1;
    inputBuffer->buffer[bytesRead - 1] = 0;
}

int main(int argc, char *argv[])
{
    InputBuffer *inputBuffer = newInputBuffer();
    while (true)
    {
        printPrompt();
        readInput(inputBuffer);

        if (strcmp(inputBuffer->buffer, ".exit") == 0)
        {
            closeInputBuffer(inputBuffer);
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Unrecognized command '%s'.\n", inputBuffer->buffer);
        }
    }
}

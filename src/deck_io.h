#ifndef DECK_IO_H
#define DECK_IO_H

#include <stddef.h>

int ReadDeckFile(
    const char *filePath,
    char *buffer,
    size_t bufferSize
);

int ExtractDeckName(
    const char *jsonText,
    char *nameBuffer,
    size_t nameBufferSize
);

int CountDeckCards(
    const char *jsonText
);

int ExtractFirstCardFields(
    const char *jsonText,
    char *frontBuffer,
    size_t frontBufferSize,
    char *backBuffer,
    size_t backBufferSize
);

#endif

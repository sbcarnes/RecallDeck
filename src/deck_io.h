#ifndef DECK_IO_H
#define DECK_IO_H

#include <stddef.h>

int ReadDeckFile(
    const char *filePath,
    char *buffer,
    size_t bufferSize
);

#endif

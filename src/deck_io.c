#include "deck_io.h"

#include <stdio.h>

int ReadDeckFile(
    const char *filePath,
    char *buffer,
    size_t bufferSize
)
{
    FILE *file = fopen(filePath, "rb");
    
    if (file == NULL)
    {
        return 0;
    }
    
    size_t bytesRead =
        fread(
            buffer, 1,
            bufferSize - 1,
            file
        );
    
    buffer[bytesRead] = '\0';
    
    fclose(file);
    
    return 1;
}
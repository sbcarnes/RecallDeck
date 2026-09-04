#include "deck_io.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

int ExtractDeckName(
    const char *jsonText,
    char *nameBuffer,
    size_t nameBufferSize
)
{
    if (jsonText == NULL ||
        nameBuffer == NULL ||
        nameBufferSize == 0)
    {
        return 0;
    }
    
    const char *nameKey = strstr(jsonText, "\"name\"");
    
    if(nameKey == NULL)
    {
        return 0;
    }
    
    const char *colon = strchr(nameKey, ':');
    
    if (colon == NULL)
    {
        return 0;
    }
    
    const char *valueStart = colon + 1;
    
    while (*valueStart != '\0' && isspace((unsigned char)*valueStart))
    {
        valueStart++;
    }
    
    if (*valueStart != '"')
    {
        return 0;
    }
    
    valueStart++;
    
    const char *valueEnd = strchr(valueStart, '"');
    
    if (valueEnd == NULL)
    {
        return 0;
    }
    
    size_t nameLength = (size_t)(valueEnd - valueStart);
    
    if (nameLength >= nameBufferSize)
    {
        return 0;
    }
    
    memcpy(nameBuffer, valueStart, nameLength);
    
    nameBuffer[nameLength] = '\0';
    
    return 1;
}
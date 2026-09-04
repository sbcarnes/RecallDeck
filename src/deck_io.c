#include "deck_io.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char *FindMatchingBrace(const char *objectStart);

/*static int ExtractJsonStringField(
    const char *objectStart,
    const char *objectEnd,
    const char *fieldName,
    char *buffer,
    size_t bufferSize
);*/

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

static const char *FindMatchingBrace(
    const char *objectStart
)
{
    if (objectStart == NULL || *objectStart != '{')
    {
        return NULL;
    }
    
    int depth = 0;
    int insideString = 0;
    int escaped = 0;
    
    const char *cursor = objectStart;
    
    while (*cursor != '\0')
    {
        char current = *cursor;
        
        if (insideString)
        {
            if (escaped)
            {
                escaped = 0;
            }
            else if (current == '\\')
            {
                escaped = 1;
            }
            else if (current == '"')
            {
                insideString = 0;
            }
        }
        else
        {
            if (current == '"')
            {
                insideString = 1;
            }
            else if (current == '{')
            {
                depth++;
                
            }
            else if (current == '}')
            {
                depth--;
                
                if (depth == 0)
                {
                    return cursor;
                }
            }
        }
        
        cursor++;
    }
    
    return NULL;
}

int CountDeckCards(
    const char *jsonText
)
{
    if (jsonText == NULL)
    {
        return -1;
    }
    
    const char *cardsKey = strstr(jsonText, "\"cards\"");
    
    if (cardsKey == NULL)
    {
        return -1;
    }
    
    const char *arrayStart = strchr(cardsKey, '[');
    
    if (arrayStart == NULL)
    {
        return -1;
    }
    
    const char *cursor = arrayStart + 1;
    
    int cardCount = 0;
    
    while (*cursor != '\0')
    {
        // Skip whitespace and commas
        // between card objects
        while(*cursor != '\0' &&
              (isspace((unsigned char)*cursor) ||
               *cursor == ','))
        {
            cursor++;
        }
        
        // Closing ] means the card
        // array is finished
        if (*cursor == ']')
        {
            return cardCount;
        }
        
        // Each array member is expected
        // to be an object
        if (*cursor != '{')
        {
            return -1;
        }
        
        const char *cardEnd = FindMatchingBrace(cursor);
        
        if (cardEnd == NULL)
        {
            return -1;
        }
        
        cardCount++;
        
        cursor = cardEnd + 1;
    }
    
    return -1;
}


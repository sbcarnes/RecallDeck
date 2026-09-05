#include "deck_io.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char *FindMatchingBrace(const char *objectStart);

static int ExtractJsonStringField(
    const char *objectStart,
    const char *objectEnd,
    const char *fieldName,
    char *buffer,
    size_t bufferSize
);

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

static int ExtractJsonStringField(
    const char *objectStart,
    const char *objectEnd,
    const char *fieldName,
    char *buffer,
    size_t bufferSize
)
{
    if (objectStart == NULL ||
        objectEnd == NULL ||
        fieldName == NULL ||
        buffer == NULL ||
        bufferSize == 0 ||
        objectEnd <= objectStart)
    {
        return 0;
    }
    
    size_t fieldNameLength = strlen(fieldName);
    
    const char *cursor = objectStart + 1;
    
    while (cursor < objectEnd)
    {
        // Skip whitespace and commas
        // between fields
        
        while (cursor < objectEnd &&
                (isspace((unsigned char)*cursor) ||
                 *cursor == ','))
        {
            cursor++;
        }
        
        if (cursor >= objectEnd)
        {
            break;
        }
        
        // Each field must begin with a quoted key
        // under current schema for RecallDeck
        if (*cursor != '"')
        {
            return 0;
        }
        
        cursor++;
        
        const char *keyStart = cursor;
        
        int escaped = 0;
        
        // Find key closing quote
        while (cursor < objectEnd)
        {
            if (escaped)
            {
                escaped = 0;
            }
            else if (*cursor == '\\')
            {
                escaped = 1;
            }
            else if (*cursor == '"')
            {
                break;
            }
            
            cursor++;
        }
        
        if (cursor >= objectEnd)
        {
            return 0;
        }
        
        const char *keyEnd = cursor;
        
        cursor++;
        
        while (cursor < objectEnd &&
               isspace((unsigned char)*cursor))
        {
            cursor++;
        }
        
        if (cursor >= objectEnd || *cursor != ':')
        {
            return 0;
        }
        
        cursor++;
        
        while (cursor < objectEnd && isspace((unsigned char)*cursor))
        {
            cursor++;
        }
        
        // Strings are expected in the fields
        // being looked for in v0
        if (cursor >= objectEnd || *cursor != '"')
        {
            return 0;
        }
        
        cursor++;
        
        int isRequestedField =
            (size_t)(keyEnd - keyStart) == fieldNameLength &&
            strncmp(keyStart, fieldName, fieldNameLength) == 0;
        
        size_t charactersWritten = 0;
        escaped = 0;
        
        // Walk through the string value
        while (cursor < objectEnd)
        {
            char current = *cursor;
            
            cursor++;
            
            if (escaped)
            {
                char decoded;
                
                switch (current)
                {
                    case '"':
                        decoded = '"';
                    break;
                    
                    case '\\':
                        decoded = '\\';
                    break;
                    
                    case '/':
                        decoded = '/';
                    break;
                    
                    case 'n':
                        decoded = '\n';
                    break;
                    
                    case 'r':
                        decoded = '\r';
                    break;
                    
                    case 't':
                        decoded = '\t';
                    break;
                    
                    case 'b':
                        decoded = '\b';
                    break;
                    
                    case 'f':
                        decoded = '\f';
                    break;
                    
                    default:
                        // Unicode \uXXXX escapes
                        // not currently supported
                        // by this parser
                        if (isRequestedField)
                        {
                            return 0;
                        }
                        
                        decoded = current;
                    break;
                }
                
                if (isRequestedField)
                {
                    if (charactersWritten + 1 >= bufferSize)
                    {
                        return 0;
                    }
                    
                    buffer[charactersWritten] = decoded;
                    
                    charactersWritten++;
                }
                
                escaped = 0;
            }
            else if (current == '\\')
            {
                escaped = 1;
            }
            else if (current == '"')
            {
                // End of field's value
                if (isRequestedField)
                {
                    buffer[charactersWritten] = '\0';
                    
                    return 1;
                }
                break;
            }
            else if (isRequestedField)
            {
                if (charactersWritten + 1 >= bufferSize)
                {
                    return 0;
                }
                
                buffer[charactersWritten] = current;
                
                charactersWritten++;
            }
        }
        
        if (escaped)
        {
            return 0;
        }
    }
    
    // Requested field not found in this object
    return 0;
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

int ExtractFirstCardFields(
    const char *jsonText,
    char *frontBuffer,
    size_t frontBufferSize,
    char *backBuffer,
    size_t backBufferSize
)
{
    if (jsonText == NULL)
    {
        return 0;
    }
    
    const char *cardsKey = strstr(jsonText, "\"cards\"");
    
    if (cardsKey == NULL)
    {
        return 0;
    }
    
    const char *arrayStart = strchr(cardsKey, '[');
    
    if (arrayStart == NULL)
    {
        return 0;
    }
    
    const char *cardStart = arrayStart + 1;
    
    while (*cardStart != '\0' && isspace((unsigned char)*cardStart))
    {
        cardStart++;
    }
    
    if (*cardStart != '{')
    {
        return 0;
    }
    
    const char *cardEnd = FindMatchingBrace(cardStart);
    
    if (cardEnd == NULL)
    {
        return 0;
    }
    
    if (!ExtractJsonStringField(
            cardStart,
            cardEnd,
            "front",
            frontBuffer,
            frontBufferSize))
    {
        return 0;
    }
    
    if (!ExtractJsonStringField(
            cardStart,
            cardEnd,
            "back",
            backBuffer,
            backBufferSize))
    {
        return 0;
    }
    
    return 1;
}


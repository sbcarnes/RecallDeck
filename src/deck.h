#ifndef DECK_H
#define DECK_H

#include <stddef.h>

#define MAX_CARDS 32

typedef struct Flashcard
{
    char id[64];
    
    char frontText[256];
    char backText[256];
    
    unsigned int hits;
    unsigned int misses;
} Flashcard;

typedef struct Deck
{
    Flashcard cards[MAX_CARDS];
    
    size_t cardCount;
    size_t currentIndex;
    
    size_t reviewOrder[MAX_CARDS];
    size_t reviewPosition;
} Deck;

#endif

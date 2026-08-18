#ifndef APP_H
#define APP_H

#include <windows.h>

#define MAX_CARDS 32

typedef enum CardSide
{
    CARD_FRONT = 0,
    CARD_BACK
} CardSide;

typedef struct Flashcard
{
    RECT bounds;
    
    BOOL isHovered;
    BOOL isPressed;
    BOOL wasDragged;
    
    POINT dragOffset;
    
    CardSide visibleSide;
    
    char frontText[256];
    char backText[256];
    
    unsigned int hits;
    unsigned int misses;
    
    RECT missedButtonRect;
    RECT gotItButtonRect;
} Flashcard;

typedef struct Deck
{
    Flashcard cards[MAX_CARDS];
    size_t cardCount;
    size_t currentIndex;
} Deck;

typedef struct AppState
{
    RECT clientRect;
    Flashcard card;
    BOOL trackingMouseLeave;
    Deck deck;
} AppState;



void InitializeApp(HWND hwnd, AppState *app);

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card
);

#endif

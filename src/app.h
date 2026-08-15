#ifndef APP_H
#define APP_H

#include <windows.h>

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
} Flashcard;

typedef struct AppState
{
    RECT clientRect;
    Flashcard card;
    BOOL trackingMouseLeave;
} AppState;

void InitializeApp(HWND hwnd, AppState *app);

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card
);

#endif

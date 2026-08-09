#ifndef APP_H
#define APP_H

#include <windows.h>

typedef struct Flashcard
{
    RECT bounds;
    
    BOOL isHovered;
    BOOL isPressed;
    
    POINT dragOffset;
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

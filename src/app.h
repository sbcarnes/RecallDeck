#ifndef APP_H
#define APP_H

#include <windows.h>

typedef struct Flashcard
{
    RECT bounds;
    BOOL isHovered;
} Flashcard;

typedef struct AppState
{
    RECT clientRect;
    Flashcard card;
} AppState;

void InitializeApp(HWND hwnd, AppState *app);

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card
);

#endif

#ifndef APP_H
#define APP_H

#include <windows.h>

#define MAX_CARDS 32

typedef enum CardSide
{
    CARD_FRONT = 0,
    CARD_BACK
} CardSide;

typedef enum CardHoverTarget
{
    CARD_HOVER_NONE = 0,
    CARD_HOVER_SURFACE,
    CARD_HOVER_MISSED,
    CARD_HOVER_GOT_IT
} CardHoverTarget;

typedef enum CardPressTarget
{
    CARD_PRESS_NONE = 0,
    CARD_PRESS_SURFACE,
    CARD_PRESS_MISSED,
    CARD_PRESS_GOT_IT
} CardPressTarget;

typedef struct FlashcardSeed
{
    const char *frontText;
    const char *backText;
} FlashcardSeed;

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
    
    CardHoverTarget hoverTarget;
    CardPressTarget pressTarget;
} Flashcard;

typedef struct Deck
{
    Flashcard cards[MAX_CARDS];
    
    size_t cardCount;
    size_t currentIndex;
    
    size_t reviewOrder[MAX_CARDS];
    size_t reviewPosition;
} Deck;

typedef struct AppState
{
    RECT clientRect;
    //Flashcard card;
    BOOL trackingMouseLeave;
    Deck deck;
} AppState;

void UpdateFlashcardHover(
    Flashcard *card,
    POINT mousePosition
);

void BeginFlashcardPress(
    Flashcard *card,
    POINT mousePosition
);

BOOL HandleFlashcardClick(
    Flashcard *card,
    POINT mousePosition
);

void InitializeApp(HWND hwnd, AppState *app);

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card
);

void DrawDiagnostics(
    HDC hdc,
    const RECT *clientRect,
    const Deck *deck
);

void AdvanceDeck(Deck *deck);

Flashcard *GetCurrentCard(
    Deck *deck
);

#endif

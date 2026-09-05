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

typedef enum AppMode
{
    APP_MODE_REVIEW = 0,
    APP_MODE_SESSION_COMPLETE
} AppMode;

typedef struct FlashcardSeed
{
    const char *frontText;
    const char *backText;
} FlashcardSeed;

typedef struct Flashcard
{
    char frontText[256];
    char backText[256];
    
    unsigned int hits;
    unsigned int misses;
} Flashcard;

typedef struct FlashcardView
{
    RECT bounds;
    
    CardSide visibleSide;
    
    BOOL isPressed;
    BOOL wasDragged;
    
    POINT dragOffset;
    
    CardPressTarget pressTarget;
    CardHoverTarget hoverTarget;
} FlashcardView;

typedef struct Deck
{
    Flashcard cards[MAX_CARDS];
    
    size_t cardCount;
    size_t currentIndex;
    
    size_t reviewOrder[MAX_CARDS];
    size_t reviewPosition;
} Deck;

typedef struct DeckLoadStatus
{
    BOOL fileLoaded;
    BOOL nameLoaded;
    BOOL cardCountLoaded;
    
    char deckName[128];
    int cardCount;
} DeckLoadStatus;

typedef struct AppState
{
    RECT clientRect;
    
    BOOL trackingMouseLeave;
    
    Deck deck;
    FlashcardView cardView;
    
    AppMode mode;
    
    DeckLoadStatus deckLoadStatus;
} AppState;

BOOL HandleSessionCompleteClick(
    AppState *app,
    const RECT *clientRect,
    POINT mousePosition
);

void UpdateFlashcardHover(
    FlashcardView *view,
    POINT mousePosition
);

void BeginFlashcardPress(
    FlashcardView *view,
    POINT mousePosition
);

BOOL HandleFlashcardClick(
    Flashcard *card,
    FlashcardView *view,
    POINT mousePosition
);

void InitializeApp(HWND hwnd, AppState *app);

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card,
    const FlashcardView *view
);

void DrawDiagnostics(
    HDC hdc,
    const RECT *clientRect,
    const Deck *deck
);

void DrawDeckLoadStatus(
    HDC hdc,
    const DeckLoadStatus *status
);

void DrawSessionComplete(
    HDC hdc,
    const RECT *clientRect
);

void DrawSessionProgress(
    HDC hdc,
    const Deck *deck
);

BOOL AdvanceDeck(Deck *deck);

Flashcard *GetCurrentCard(
    Deck *deck
);

#endif

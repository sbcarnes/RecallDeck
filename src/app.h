#ifndef APP_H
#define APP_H

#include <windows.h>
#include "deck.h"

/* =========================================================
   Types
   ========================================================= */

/* enums */

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
    APP_MODE_DECK_SELECT,
    APP_MODE_EDITOR
} AppMode;

typedef enum ReviewMode
{
    REVIEW_MODE_ACTIVE = 0,
    REVIEW_MODE_SESSION_COMPLETE,
    REVIEW_MODE_NO_CARDS_DUE
} ReviewMode;

typedef enum CardReviewResult
{
    CARD_REVIEW_NONE = 0,
    CARD_REVIEW_MISSED,
    CARD_REVIEW_GOT_IT
} CardReviewResult;

typedef enum ReviewSessionType
{
    REVIEW_SESSION_SCHEDULED = 0,
    REVIEW_SESSION_PRACTICE
} ReviewSessionType;

/* structs */

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

typedef struct EditorView
{
    HWND questionEdit;
    HWND answerEdit;
    
    BOOL hasSelection;
    size_t selectedCardIndex;
} EditorView;

typedef struct DeckLoadStatus
{
    BOOL fileLoaded;
    BOOL nameLoaded;
    BOOL cardCountLoaded;
    BOOL progressLoaded;
    
    char deckName[128];
    int cardCount;
} DeckLoadStatus;

typedef struct AppState
{
    RECT clientRect;
    
    BOOL trackingMouseLeave;
    
    Deck deck;
    FlashcardView cardView;
    EditorView editorView;
    
    AppMode mode;
    
    ReviewMode reviewMode;
    
    DeckLoadStatus deckLoadStatus;
    
    int diagnosticScrollOffset;
    
    ReviewSessionType reviewSessionType;
} AppState;

/* =========================================================
   Initialization
   ========================================================= */

void InitializeApp(HWND hwnd, AppState *app);

BOOL InitializeEditorControls(
    HWND hwnd,
    AppState *app
);

/* =========================================================
   Deck and review state
   ========================================================= */

Flashcard *GetCurrentCard(
    Deck *deck
);

BOOL AdvanceDeck(Deck *deck);

void QueueRetryCard(
    Deck *deck,
    size_t cardIndex
);

void RecordReviewResult(
    AppState *app,
    CardReviewResult result
);

int SaveCurrentProgress(
    const AppState *app
);

/* =========================================================
   Interaction
   ========================================================= */

void UpdateFlashcardHover(
    FlashcardView *view,
    POINT mousePosition
);

void BeginFlashcardPress(
    FlashcardView *view,
    POINT mousePosition
);

CardReviewResult HandleFlashcardClick(
    FlashcardView *view,
    POINT mousePosition
);

BOOL HandleSessionCompleteClick(
    AppState *app,
    const RECT *clientRect,
    POINT mousePosition
);

BOOL HandleNoCardsDueClick(
    AppState *app,
    const RECT *clientRect,
    POINT mousePosition
);

BOOL HandleDiagnosticsScroll(
    AppState *app,
    const RECT *clientRect,
    POINT mousePosition,
    int wheelDelta
);

BOOL HandleEditorCardClick(
    AppState *app,
    const RECT *clientRect,
    POINT mousePosition
);

BOOL HandleModeTabClick(
    AppState *app,
    POINT mousePosition
);

void UpdateEditorControls(
    HWND hwnd,
    AppState *app
);

/* =========================================================
   Drawing
   ========================================================= */

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card,
    const FlashcardView *view
);

void DrawSessionProgress(
    HDC hdc,
    const Deck *deck,
    const RECT *clientRect
);

void DrawSessionComplete(
    HDC hdc,
    const RECT *clientRect
);

void DrawNoCardsDue(
    HDC hdc,
    const RECT *clientRect
);

void DrawDiagnostics(
    HDC hdc,
    const RECT *clientRect,
    const Deck *deck,
    int scrollOffset
);

void DrawDeckLoadStatus(
    HDC hdc,
    const DeckLoadStatus *status
);

void DrawModeTabs(
    HDC hdc,
    AppMode mode
);

void DrawEditorShell(
    HDC hdc,
    const RECT *clientRect,
    const Deck *deck,
    const EditorView *editor
);

#endif

#include "app.h"
#include <stdio.h>

static void DrawCardBackground(HDC hdc, const Flashcard *card);
static void DrawCardText(HDC hdc,  const Flashcard *card);
static void DrawRevealHint(HDC hdc, const Flashcard *card);
static void DrawAnswerControls(HDC hdc, const Flashcard *card);

static const FlashcardSeed starterCards[] =
{
    {
        "What message reports mouse movement?",
        "WM_MOUSEMOVE"
    },
    {
        "What message is sent when a window needs repainting?",
        "WM_PAINT"
    },
    {
        "What message is sent when the left mouse button is pressed?",
        "WM_LBUTTONDOWN"
    }
};

static void InitializeFlashcard(
    Flashcard *card,
    const char *frontText,
    const char *backText
)
{
    SetRect(
        &card->bounds,
        80,
        80,
        420,
        280
    );
    
    card->visibleSide = CARD_FRONT;
    
    card->isPressed = FALSE;
    card->wasDragged = FALSE;
    
    card->pressTarget = CARD_PRESS_NONE;
    card->hoverTarget = CARD_HOVER_NONE;
    
    card->hits = 0;
    card-> misses = 0;
    
    snprintf(
        card->frontText,
        sizeof(card->frontText),
        "%s",
        frontText
    );
    
    snprintf(
        card->backText,
        sizeof(card->backText),
        "%s",
        backText
    );
}

static void GetReviewAgainButtonRect(
    const RECT *clientRect,
    RECT *buttonRect
)
{
    int buttonWidth = 160;
    int buttonHeight = 40;
    
    int centerX =
        (clientRect->left + clientRect->right) / 2;
    
    int centerY =
        (clientRect->top + clientRect->bottom) / 2;
    
    SetRect(
        buttonRect,
        centerX - buttonWidth / 2, centerY + 30,
        centerX + buttonWidth / 2, centerY + 30 + buttonHeight
    );
}

// Fisher-Yates shuffle (TODO look up later)
static void ShuffleReviewOrder(
    Deck *deck
)
{
    if (deck->cardCount < 2)
    {
        return;
    }
    
    for (size_t i = deck->cardCount -1; i > 0; i--)
    {
        size_t j = (size_t)(rand() % (i + 1));
        
        size_t temp = deck->reviewOrder[i];
        
        deck->reviewOrder[i] = deck->reviewOrder[j];
        
        deck->reviewOrder[j] = temp;
    }
    
    deck->reviewPosition = 0;
    
    deck->currentIndex = deck->reviewOrder[0];
}

void RestartReviewSession(
    AppState *app
)
{
    Deck *deck = &app->deck;
    
    if (deck->cardCount == 0)
    {
        return;
    }
    
    ShuffleReviewOrder(deck);
    
    deck->reviewPosition = 0;
    
    deck->currentIndex = deck->reviewOrder[0];
    
    app->mode = APP_MODE_REVIEW;
}

BOOL HandleSessionCompleteClick(
    AppState *app,
    const RECT *clientRect,
    POINT mousePosition
)
{
    RECT buttonRect;
    
    GetReviewAgainButtonRect(
        clientRect,
        &buttonRect
    );
    
    if (!PtInRect(
        &buttonRect, mousePosition
    ))
    {
        return FALSE;
    }
    
    RestartReviewSession(app);
    
    return TRUE;
}

static void GetAnswerButtonRects(
    const Flashcard *card,
    RECT *missedRect,
    RECT *gotItRect
)
{
    int cardWidth = card->bounds.right - card->bounds.left;
    int cardHeight = card->bounds.bottom - card->bounds.top;
    
    int columnLeft = card->bounds.left + (cardWidth * 2 / 3);
    
    int buttonMargin = 16;
    int buttonGap = 14;
    
    int buttonLeft = columnLeft + buttonMargin;
    int buttonRight = card->bounds.right - buttonMargin;
    
    int availableHeight = cardHeight - (buttonMargin * 2);
    int buttonHeight = (availableHeight - buttonGap) / 2;
    
    SetRect(
        missedRect,
        buttonLeft,
        card->bounds.top + buttonMargin,
        buttonRight,
        card->bounds.top + buttonMargin + buttonHeight
    );
    
    SetRect(
        gotItRect,
        buttonLeft,
        card->bounds.top + buttonMargin + buttonHeight + buttonGap,
        buttonRight,
        card->bounds.bottom - buttonMargin
    );
}

static void DrawCardBackground(
    HDC hdc,
    const Flashcard *card
)
{
    COLORREF cardColor;
    
    if (card->visibleSide == CARD_FRONT)
    {
        /* subtle cool blue-gray */
        if (card->pressTarget == CARD_PRESS_SURFACE)
        {
            cardColor = RGB(210, 220, 228);
        }
        else if (card->hoverTarget == CARD_HOVER_SURFACE)
        {
            cardColor = RGB(220, 230, 238);
        }
        else
        {
            cardColor = RGB(235, 242, 248);
        }
    }
    else
    {
        /* subtle warm green-gray */
        if (card->pressTarget == CARD_PRESS_SURFACE)
        {
            cardColor = RGB(216, 222, 207);
        }
        else if (card->hoverTarget == CARD_HOVER_SURFACE)
        {
            cardColor = RGB(228, 233, 219);
        }
        else
        {
            cardColor = RGB(240, 244, 232);
        }
    }
    
    HBRUSH cardBrush = CreateSolidBrush(cardColor);
    
    if (cardBrush == NULL)
    {
        return;
    }
    
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, cardBrush);
    
    HPEN oldPen = 
        (HPEN)SelectObject(hdc, GetStockObject(BLACK_PEN));
    
    Rectangle(
        hdc,
        card->bounds.left,
        card->bounds.top,
        card->bounds.right,
        card->bounds.bottom
    );
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    
    DeleteObject(cardBrush);
}

static void DrawCardText(
    HDC hdc,
    const Flashcard *card
)
{
    const char *cardText =
        card->visibleSide == CARD_FRONT
            ? card->frontText
            : card->backText;
    
    RECT contentRect = card->bounds;
    
    contentRect.left += 24;
    contentRect.top += 24;
    
    /*
        Leave extra room underneath for:
            - reveal hint on the front
            - grading buttons on the back
    */
    
    if (card-> visibleSide == CARD_FRONT)
    {
        contentRect.right -= 24;
        
        // Reserve lower area for reveal hint
        
        contentRect.bottom -= 70;
    }
    else
    {
        int cardWidth = card->bounds.right - card->bounds.left;
        
        // Stop answer text before right-hand button column
        
        contentRect.right =
            card->bounds.left + (cardWidth * 2 / 3) - 16;
        
        contentRect.bottom -= 24;
    }
    
    RECT measuredRect = contentRect;
    
    DrawText(
        hdc,
        cardText,
        -1,
        &measuredRect,
        DT_CENTER | DT_WORDBREAK | DT_CALCRECT
    );
    
    int textHeight =
        measuredRect.bottom - measuredRect.top;
    
    int availableHeight =
        contentRect.bottom - contentRect.top;
    
    if (textHeight < availableHeight)
    {
        contentRect.top +=
            (availableHeight - textHeight) / 2;
    }
    
    int oldBackgroundMode = SetBkMode(hdc, TRANSPARENT);
    
    DrawText(
        hdc,
        cardText,
        -1,
        &contentRect,
        DT_CENTER | DT_WORDBREAK
    );
    
    SetBkMode(
        hdc, oldBackgroundMode
    );
}

static void DrawRevealHint(
    HDC hdc,
    const Flashcard *card
)
{
    RECT hintRect = card->bounds;
    
    hintRect.left += 20;
    hintRect.right -= 20;
    
    hintRect.bottom -= 12;
    hintRect.top = hintRect.bottom - 24;
    
    COLORREF oldTextColor =
        SetTextColor(
            hdc,
            RGB(110, 110, 110)
        );
    
    int oldBackgroundMode =
        SetBkMode(
            hdc, TRANSPARENT
        );
    
    DrawText(
        hdc,
        "Click to reveal",
        -1,
        &hintRect,
        DT_RIGHT | DT_SINGLELINE
    );
    
    SetBkMode(
        hdc,
        oldBackgroundMode
    );
    
    SetTextColor(
        hdc,
        oldTextColor
    );
}

static void DrawAnswerControls(
    HDC hdc,
    const Flashcard *card
)
{
    RECT missedRect;
    RECT gotItRect;
    
    GetAnswerButtonRects(
        card,
        &missedRect,
        &gotItRect
    );
    
    COLORREF missedColor;
    COLORREF gotItColor;
    
    if (card->pressTarget == CARD_PRESS_MISSED)
    {
        missedColor = RGB(155, 65, 65);
    }
    else if (card->hoverTarget == CARD_HOVER_MISSED)
    {
        missedColor = RGB(170, 75, 75);
    }
    else
    {
        missedColor = RGB(185, 85, 85);
    }
    
    if (card->pressTarget == CARD_PRESS_GOT_IT)
    {
        gotItColor = RGB(50, 100, 60);
    }
    else if (card->hoverTarget == CARD_HOVER_GOT_IT)
    {
        gotItColor = RGB(60, 112, 70);
    }
    else
    {
        gotItColor = RGB(70, 125, 80);
    }
    
    HBRUSH missedBrush = CreateSolidBrush(missedColor);
    HBRUSH gotItBrush = CreateSolidBrush(gotItColor);
    
    if (missedBrush == NULL || gotItBrush == NULL)
    {
        if (missedBrush != NULL)
        {
            DeleteObject(missedBrush);
        }
        
        if (gotItBrush != NULL)
        {
            DeleteObject(gotItBrush);
        }
        
        return;
    }
    
    HPEN oldPen =
        (HPEN)SelectObject(
            hdc,
            GetStockObject(BLACK_PEN)
        );
    
    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            missedBrush
        );
    
    Rectangle(
        hdc,
        missedRect.left,
        missedRect.top,
        missedRect.right,
        missedRect.bottom
    );
    
    SelectObject(hdc, gotItBrush);
    
    Rectangle(
        hdc,
        gotItRect.left,
        gotItRect.top,
        gotItRect.right,
        gotItRect.bottom
    );
    
    COLORREF oldTextColor = 
        SetTextColor(
            hdc,
            RGB(255, 255, 255)
        );
    
    int oldBackgroundMode =
        SetBkMode(
            hdc,
            TRANSPARENT
        );
    
    DrawText(
        hdc,
        "Missed",
        -1,
        &missedRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );
    
    DrawText(
        hdc,
        "Got It",
        -1,
        &gotItRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );
    
    SetBkMode(
        hdc,
        oldBackgroundMode
    );
    
    SetTextColor(
        hdc,
        oldTextColor
    );
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    
    DeleteObject(missedBrush);
    DeleteObject(gotItBrush);
}

void UpdateFlashcardHover(
    Flashcard *card,
    POINT mousePosition
)
{
    card->hoverTarget = CARD_HOVER_NONE;
    
    if (!PtInRect(&card->bounds, mousePosition))
    {
        return;
    }
    
    card->hoverTarget = CARD_HOVER_SURFACE;
    
    if (card->visibleSide == CARD_BACK)
    {
        RECT missedRect;
        RECT gotItRect;
        
        GetAnswerButtonRects(
            card,
            &missedRect,
            &gotItRect
        );
        
        if (PtInRect(&missedRect, mousePosition))
        {
            card->hoverTarget = CARD_HOVER_MISSED;
        }
        else if (PtInRect(&gotItRect, mousePosition))
        {
            card->hoverTarget = CARD_HOVER_GOT_IT;
        }
    }
}

void BeginFlashcardPress(
    Flashcard *card,
    POINT mousePosition
)
{
    card->pressTarget = CARD_PRESS_SURFACE;
    
    if (card->visibleSide == CARD_BACK)
    {
        RECT missedRect;
        RECT gotItRect;
        
        GetAnswerButtonRects(
            card,
            &missedRect,
            &gotItRect
        );
        
        if (PtInRect(&missedRect, mousePosition))
        {
            card->pressTarget = CARD_PRESS_MISSED;
        }
        else if (PtInRect(&gotItRect, mousePosition))
        {
            card->pressTarget = CARD_PRESS_GOT_IT;
        }
    }
}

BOOL HandleFlashcardClick(
    Flashcard *card,
    POINT mousePosition
)
{
    if (card->visibleSide == CARD_FRONT)
    {
        card->visibleSide = CARD_BACK;
        return FALSE;
    }
    
    RECT missedRect;
    RECT gotItRect;
    
    GetAnswerButtonRects(
        card,
        &missedRect,
        &gotItRect
    );
    
    if (PtInRect(&missedRect, mousePosition))
    {
        card->misses++;
        return TRUE;
    }
    else if (PtInRect(&gotItRect, mousePosition))
    {
        card->hits++;
        return TRUE;
    }
    
    return FALSE;
}

void InitializeApp(HWND hwnd, AppState *app)
{
    GetClientRect(hwnd, &app->clientRect);
    
    app->deck.cardCount =
        sizeof(starterCards) /
        sizeof(starterCards[0]);
    
    app->deck.currentIndex = 0;
    
    app->mode = APP_MODE_REVIEW;
    
    for (size_t i = 0; i < app->deck.cardCount; i++)
    {
        InitializeFlashcard(
            &app->deck.cards[i],
            starterCards[i].frontText,
            starterCards[i].backText
        );
        
        app->deck.reviewOrder[i] = i;
    }
    
    if (app->deck.cardCount > 0)
    {
        app->deck.currentIndex = app->deck.reviewOrder[0];
    }
    
    ShuffleReviewOrder(&app->deck);
    
    
}

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card
)
{
    DrawCardBackground(hdc, card);
    
    DrawCardText(hdc, card);
    
    if (card->visibleSide == CARD_FRONT)
    {
        DrawRevealHint(hdc, card);
    }
    else
    {
        DrawAnswerControls(hdc, card);
    }
    
}

BOOL AdvanceDeck(Deck *deck)
{
    if (deck->cardCount == 0)
    {
        return FALSE;
    }
    
    if (deck->reviewPosition + 1 >= deck->cardCount)
    {
        return FALSE;
    }
    
    deck->reviewPosition++;
    
    deck->currentIndex = deck->reviewOrder[deck->reviewPosition];
    
    return TRUE;
}

Flashcard *GetCurrentCard(
    Deck *deck
)
{
    return &deck->cards[deck->currentIndex];
}



void DrawDiagnostics(
    HDC hdc,
    const RECT *clientRect,
    const Deck *deck
)
{
    RECT panelRect;
    
    int panelWidth = 240;
    int panelHeight = 130;
    int margin = 20;
    
    SetRect(
        &panelRect,
        clientRect->right - panelWidth - margin,
        margin,
        clientRect->right - margin,
        margin + panelHeight
    );
    
    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            GetStockObject(LTGRAY_BRUSH)
        );
    
    HPEN oldPen = 
        (HPEN)SelectObject(
            hdc, GetStockObject(BLACK_PEN)
        );
    
    Rectangle(
        hdc,
        panelRect.left,
        panelRect.top,
        panelRect.right,
        panelRect.bottom
    );
    
    RECT textRect = panelRect;
    
    InflateRect(
        &textRect,
        -12,
        -12
    );
    
    char diagnosticText[256];
    
    int used = snprintf(
        diagnosticText,
        sizeof(diagnosticText),
        "CARD DEBUG\n\n"
    );
    
    for (size_t i = 0; i < deck->cardCount; i++)
    {
        if (used < 0 || (size_t)used >= sizeof(diagnosticText))
        {
            break;
        }
        
        const Flashcard *card = &deck->cards[i];
        
        int added = snprintf(
            diagnosticText + used,
            sizeof(diagnosticText) - (size_t)used,
            "%c [%zu] Hits: %u Misses: %u\n",
            i == deck->currentIndex ? '>' : ' ',
            i, card->hits, card->misses
        );
        
        if (added < 0)
        {
            break;
        }
        
        used += added;
    }
    
    int oldBackgroundMode =
        SetBkMode(
            hdc,
            TRANSPARENT
        );
    
    DrawText(
        hdc,
        diagnosticText,
        -1, &textRect,
        DT_LEFT | DT_TOP
    );
    
    SetBkMode(hdc, oldBackgroundMode);
    
    SelectObject(hdc, oldPen);
    
    SelectObject(hdc, oldBrush);
}

void DrawSessionComplete(
    HDC hdc,
    const RECT *clientRect
)
{
    RECT messageRect = *clientRect;
    
    messageRect.bottom = (clientRect->top + clientRect->bottom) / 2;
    
    DrawText(
        hdc,
        "Review Complete",
        -1,
        &messageRect,
        DT_CENTER | DT_BOTTOM | DT_SINGLELINE
    );
    
    RECT buttonRect;
    
    GetReviewAgainButtonRect(
        clientRect,
        &buttonRect
    );
    
    HBRUSH oldBrush =
        (HBRUSH)SelectObject(
            hdc,
            GetStockObject(LTGRAY_BRUSH)
        );
    
    Rectangle(
        hdc,
        buttonRect.left,
        buttonRect.top,
        buttonRect.right,
        buttonRect.bottom
    );
    
    DrawText(
        hdc,
        "Review Again",
        -1,
        &buttonRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );
    
    SelectObject(hdc, oldBrush);
}

void DrawSessionProgress(
    HDC hdc,
    const RECT *clientRect,
    const Deck *deck
)
{
    if (deck->cardCount == 0)
    {
        return;
    }
    
    RECT progressRect;
    
    SetRect(
        &progressRect,
        20, 20, 180, 50
    );
    
    char progressText[64];
    
    snprintf(
        progressText,
        sizeof(progressText),
        "Card %zu of %zu",
        deck->reviewPosition + 1,
        deck->cardCount
    );
    
    int oldBackgroundMode =
        SetBkMode(hdc, TRANSPARENT);
    
    DrawText(hdc,
        progressText,
        -1,
        &progressRect,
        DT_LEFT | DT_VCENTER | DT_SINGLELINE
    );
    
    SetBkMode(hdc, oldBackgroundMode);
}
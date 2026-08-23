#include "app.h"
#include <stdio.h>

static void DrawCardBackground(HDC hdc, const Flashcard *card);
static void DrawCardText(HDC hdc,  const Flashcard *card);
static void DrawRevealHint(HDC hdc, const Flashcard *card);
static void DrawAnswerControls(HDC hdc, const Flashcard *card);

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
    
    COLORREF missedColor =
        card->pressTarget == CARD_PRESS_MISSED
            ? RGB(155, 65, 65)
            : RGB(185, 85, 85);
    
    COLORREF gotItColor =
        card->pressTarget == CARD_PRESS_GOT_IT
            ? RGB(50, 100, 60)
            : RGB(70, 125, 80);
    
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

void HandleFlashcardClick(
    Flashcard *card,
    POINT mousePosition
)
{
    if (card->visibleSide == CARD_FRONT)
    {
        card->visibleSide = CARD_BACK;
        return;
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
        card->visibleSide = CARD_FRONT;
    }
    else if (PtInRect(&gotItRect, mousePosition))
    {
        card->hits++;
        card->visibleSide = CARD_FRONT;
    }
    
}

void InitializeApp(HWND hwnd, AppState *app)
{
    GetClientRect(hwnd, &app->clientRect);
    
    SetRect(
        &app->card.bounds,
        80,
        80,
        420,
        280
    );
    
    app->card.isHovered = FALSE;
    app->card.isPressed = FALSE;
    
    app->card.visibleSide = CARD_FRONT;
    app->card.wasDragged = FALSE;
    
    app->card.hits = 0;
    app->card.misses = 0;
    
    app->card.pressTarget = CARD_PRESS_NONE;
    
    snprintf(
        app->card.frontText,
        sizeof(app->card.frontText),
        "What message reports mouse movement?"
    );
            
    snprintf(
        app->card.backText,
        sizeof(app->card.backText),
        "WM_MOUSEMOVE"
    );
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

void DrawDiagnostics(
    HDC hdc,
    const RECT *clientRect,
    const Flashcard *card
)
{
    RECT panelRect;
    
    int panelWidth = 180;
    int panelHeight = 110;
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
    
    char diagnosticText[128];
    
    snprintf(
        diagnosticText,
        sizeof(diagnosticText),
        "CARD DEBUG\n\nHits: %u\nMisses: %u",
        card->hits,
        card->misses
    );
    
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
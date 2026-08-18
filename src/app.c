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
    int buttonWidth = 100;
    int buttonHeight = 32;
    int gap = 16;
    
    int totalWidth = buttonWidth * 2 + gap;
    
    int startX =
        card->bounds.left +
        ((card->bounds.right - card->bounds.left) - totalWidth) / 2;
    
    int buttonTop = card->bounds.bottom - 50;
    
    SetRect(
        missedRect,
        startX,
        buttonTop,
        startX + buttonWidth,
        buttonTop + buttonHeight
    );
    
    SetRect(
        gotItRect,
        startX + buttonWidth + gap,
        buttonTop,
        startX + buttonWidth + gap + buttonWidth,
        buttonTop + buttonHeight    
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
        cardColor = RGB(235, 242, 248); /* subtle cool blue-gray */
    }
    else
    {
        cardColor = RGB(240, 244, 232); /* subtle warm green-gray */
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
    contentRect.right -= 24;
    contentRect.top += 24;
    
    /*
        Leave extra room underneath for:
            - reveal hint on the front
            - grading buttons on the back
    */
    contentRect.bottom -= 70;
    
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
    
    HBRUSH oldBrush = 
        (HBRUSH)SelectObject(
            hdc,
            GetStockObject(WHITE_BRUSH)
        );
    
    HPEN oldPen =
        (HPEN)SelectObject(
            hdc,
            GetStockObject(BLACK_PEN)
        );
    
    Rectangle(
        hdc,
        missedRect.left,
        missedRect.top,
        missedRect.right,
        missedRect.bottom
    );
    
    Rectangle(
        hdc,
        gotItRect.left,
        gotItRect.top,
        gotItRect.right,
        gotItRect.bottom
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
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
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
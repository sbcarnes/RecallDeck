#include "app.h"
#include <stdio.h>

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
    HBRUSH cardBrush;
    
    if (card->isPressed)
    {
        cardBrush = GetStockObject(GRAY_BRUSH);
    }
    else if (card->isHovered)
    {
        cardBrush = GetStockObject(LTGRAY_BRUSH);
    }
    else
    {
        cardBrush = GetStockObject(WHITE_BRUSH);
    }
    
    HBRUSH oldBrush = (HBRUSH)SelectObject(
        hdc,
        cardBrush
    );
    
    HPEN oldPen = (HPEN)SelectObject(
        hdc,
        GetStockObject(BLACK_PEN)
    );
    
    Rectangle(
        hdc,
        card->bounds.left,
        card->bounds.top,
        card->bounds.right,
        card->bounds.bottom
    );
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    
    RECT textRect = card->bounds;
    
    int oldBackgroundMode = SetBkMode(hdc, TRANSPARENT);
    
    const char *cardText =
        card->visibleSide == CARD_FRONT
            ? card->frontText
            : card->backText;
    
    DrawText(
        hdc,
        cardText,
        -1,
        &textRect,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK
    );
    
    SetBkMode(hdc, oldBackgroundMode);
}
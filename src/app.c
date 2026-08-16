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
    
    /*if (card->isPressed)
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
    );*/
    
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
    
    textRect.left += 20;
    textRect.right -= 20;
    textRect.top += 20;
    textRect.bottom -= 40;
    
    
    
    const char *cardText =
        card->visibleSide == CARD_FRONT
            ? card->frontText
            : card->backText;
    
    int oldBackgroundMode = SetBkMode(hdc, TRANSPARENT);
    
    DrawText(
        hdc,
        cardText,
        -1,
        &textRect,
        DT_CENTER | DT_VCENTER | DT_WORDBREAK
    );
    
    SetBkMode(hdc, oldBackgroundMode);
}
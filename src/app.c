#include "app.h"

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
}

void DrawFlashcard(
    HDC hdc,
    const Flashcard *card
)
{
    HBRUSH oldBrush = (HBRUSH)SelectObject(
        hdc,
        GetStockObject(WHITE_BRUSH)
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
    
    DrawText(
        hdc,
        "Front of card",
        -1,
        &textRect,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE
    );
    
    SetBkMode(hdc, oldBackgroundMode);
}
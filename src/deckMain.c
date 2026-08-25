#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "app.h"

const char g_szClassName[] = "myWindowClass";


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static AppState app;
    
    
	switch(msg)
	{
		case WM_CREATE:
            InitializeApp(hwnd, &app);
            
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            HDC memDC = CreateCompatibleDC(hdc);
            
            Flashcard *card = GetCurrentCard(&app.deck);
            
            HBITMAP memBitmap = CreateCompatibleBitmap(
                hdc,
                clientRect.right,
                clientRect.bottom
            );
            
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
            
            FillRect(
                memDC,
                &clientRect,
                (HBRUSH)(COLOR_WINDOW + 1)
            );
            
            DrawFlashcard(memDC, card);
            
            DrawDiagnostics(memDC, &clientRect, &app.deck);
            
            BitBlt(
                hdc,
                ps.rcPaint.left, ps.rcPaint.top,
                ps.rcPaint.right - ps.rcPaint.left,
                ps.rcPaint.bottom - ps.rcPaint.top,
                memDC,
                ps.rcPaint.left, ps.rcPaint.top,
                SRCCOPY
            );
            
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
        }
        break;
        case WM_MOUSEMOVE:
        {
            POINT mousePosition =
            {
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam)
            };
            
            Flashcard *card = GetCurrentCard(&app.deck);
            
            if (!app.trackingMouseLeave)
            {
                TRACKMOUSEEVENT tme = {0};
                
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                
                if (TrackMouseEvent(&tme))
                {
                    app.trackingMouseLeave = TRUE;
                }
            }
            
            if (card->isPressed && card->pressTarget == CARD_PRESS_SURFACE)
            {
                RECT oldBounds = card->bounds;
                
                int newLeft = mousePosition.x - card->dragOffset.x;
                
                int newTop = mousePosition.y - card->dragOffset.y;
                
                int deltaX = newLeft - card->bounds.left;
                
                int deltaY = newTop - card->bounds.top;
                
                
                if (deltaX != 0 || deltaY != 0)
                {
                    card->wasDragged = TRUE;
                }
                OffsetRect(&card->bounds, deltaX, deltaY);
                InvalidateRect(hwnd, &oldBounds, FALSE);
                InvalidateRect(hwnd, &card->bounds, FALSE);
            }
            else
            {
                
                CardHoverTarget oldHover = card->hoverTarget;
                
                UpdateFlashcardHover(
                    card,
                    mousePosition
                );
                
                if (card->hoverTarget != oldHover)
                {
                    InvalidateRect(
                        hwnd,
                        &card->bounds,
                        FALSE
                    );
                }
            }
            
            
        }
        break;
        case WM_LBUTTONDOWN:
        {
            POINT mousePosition =
            {
              GET_X_LPARAM(lParam),
              GET_Y_LPARAM(lParam)
            };
            
            Flashcard *card = GetCurrentCard(&app.deck);
            
            if (PtInRect(&card->bounds, mousePosition))
            {
                BeginFlashcardPress(card, mousePosition);
                
                card->isPressed = TRUE;
                card->wasDragged = FALSE;
                
                card->dragOffset.x =
                    mousePosition.x - card->bounds.left;
                
                card->dragOffset.y =
                    mousePosition.y - card->bounds.top;
                    
                SetCapture(hwnd);
                
                InvalidateRect(
                    hwnd,
                    &card->bounds,
                    FALSE
                );
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            Flashcard *card = GetCurrentCard(&app.deck);
            
            if (card->isPressed)
            {
                POINT mousePosition =
                {
                    GET_X_LPARAM(lParam),
                    GET_Y_LPARAM(lParam)
                };
                
                BOOL isCardClick =
                    !card->wasDragged &&
                    PtInRect(&card->bounds, mousePosition);
                
                card->isPressed = FALSE;
                
                ReleaseCapture();
                
                if (isCardClick)
                {
                    BOOL reviewCompleted = HandleFlashcardClick(
                        card,
                        mousePosition
                    );
                    
                    if (reviewCompleted)
                    {
                        card->visibleSide = CARD_FRONT;
                        
                        AdvanceDeck(
                            &app.deck
                        );
                    }
                    
                    InvalidateRect(
                        hwnd,
                        NULL,
                        FALSE
                    );
                }
                else
                {
                    InvalidateRect(
                        hwnd,
                        &card->bounds,
                        FALSE
                    );
                }
                
                card->pressTarget = CARD_PRESS_NONE;
                card->isPressed = FALSE;
                card->wasDragged = FALSE;
                
                
            }
        }
        break;
        case WM_MOUSELEAVE:
        {
            Flashcard *card = GetCurrentCard(&app.deck);
            
            CardHoverTarget oldHover = card->hoverTarget;
            
            app.trackingMouseLeave = FALSE;
            
            card->hoverTarget = CARD_HOVER_NONE;
            
            if (oldHover != CARD_HOVER_NONE)
            {
                InvalidateRect(
                    hwnd,
                    &card->bounds,
                    FALSE
                );
            }
        }
        break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
            PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		"Recall Deck",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 500,
		NULL, NULL, hInstance, NULL);

	if(hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
            
            if (app.mode == APP_MODE_REVIEW)
            {
                DrawFlashcard(memDC, GetCurrentCard(&app.deck), &app.cardView);
                
                DrawSessionProgress(memDC, &app.deck);
            }
            else
            {
                DrawSessionComplete(memDC, &clientRect);
            }
            
            DrawDiagnostics(memDC, &clientRect, &app.deck);
            DrawDeckLoadStatus(memDC, &app.deckLoadStatus);
            
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
            if (app.mode != APP_MODE_REVIEW)
            {
                break;
            }
            POINT mousePosition =
            {
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam)
            };
            
            FlashcardView *view = &app.cardView;
            
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
            
            if (view->isPressed && view->pressTarget == CARD_PRESS_SURFACE)
            {
                RECT oldBounds = view->bounds;
                
                int newLeft = mousePosition.x - view->dragOffset.x;
                
                int newTop = mousePosition.y - view->dragOffset.y;
                
                int deltaX = newLeft - view->bounds.left;
                
                int deltaY = newTop - view->bounds.top;
                
                
                if (deltaX != 0 || deltaY != 0)
                {
                    view->wasDragged = TRUE;
                }
                OffsetRect(&view->bounds, deltaX, deltaY);
                InvalidateRect(hwnd, &oldBounds, FALSE);
                InvalidateRect(hwnd, &view->bounds, FALSE);
            }
            else
            {
                
                CardHoverTarget oldHover = view->hoverTarget;
                
                UpdateFlashcardHover(
                    view,
                    mousePosition
                );
                
                if (view->hoverTarget != oldHover)
                {
                    InvalidateRect(
                        hwnd,
                        &view->bounds,
                        FALSE
                    );
                }
            }
            
            
        }
        break;
        case WM_LBUTTONDOWN:
        {
            if (app.mode != APP_MODE_REVIEW)
            {
                break;
            }
            
            POINT mousePosition =
            {
              GET_X_LPARAM(lParam),
              GET_Y_LPARAM(lParam)
            };
            
            FlashcardView *view = &app.cardView;
            
            if (PtInRect(&view->bounds, mousePosition))
            {
                BeginFlashcardPress(view, mousePosition);
                
                view->isPressed = TRUE;
                view->wasDragged = FALSE;
                
                view->dragOffset.x =
                    mousePosition.x - view->bounds.left;
                
                view->dragOffset.y =
                    mousePosition.y - view->bounds.top;
                    
                SetCapture(hwnd);
                
                InvalidateRect(
                    hwnd,
                    &view->bounds,
                    FALSE
                );
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            POINT mousePosition =
            {
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam)
            };
            
            if (app.mode == APP_MODE_SESSION_COMPLETE)
            {
                RECT clientRect;
                
                GetClientRect(hwnd, &clientRect);
                
                if (HandleSessionCompleteClick(
                    &app, &clientRect, mousePosition
                ))
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                
                break;
            }
            
            if (app.mode != APP_MODE_REVIEW)
            {
                break;
            }
            
            Flashcard *card = GetCurrentCard(&app.deck);
            FlashcardView *view = &app.cardView;
            
            if (view->isPressed)
            {
                BOOL isCardClick =
                    !view->wasDragged &&
                    PtInRect(&view->bounds, mousePosition);
                
                view->isPressed = FALSE;
                
                ReleaseCapture();
                
                if (isCardClick)
                {
                    BOOL reviewCompleted = HandleFlashcardClick(
                        card, view,
                        mousePosition
                    );
                    
                    if (reviewCompleted)
                    {
                        view->visibleSide = CARD_FRONT;
                        
                        if (!AdvanceDeck(&app.deck))
                        {
                            app.mode = APP_MODE_SESSION_COMPLETE;
                        }
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
                        &view->bounds,
                        FALSE
                    );
                }
                
                view->pressTarget = CARD_PRESS_NONE;
                view->isPressed = FALSE;
                view->wasDragged = FALSE;
            }
        }
        break;
        case WM_MOUSELEAVE:
        {
            FlashcardView *view = &app.cardView;
            
            CardHoverTarget oldHover = view->hoverTarget;
            
            app.trackingMouseLeave = FALSE;
            
            view->hoverTarget = CARD_HOVER_NONE;
            
            if (oldHover != CARD_HOVER_NONE)
            {
                InvalidateRect(
                    hwnd,
                    &view->bounds,
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
    
    srand((unsigned int)time(NULL));
    
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
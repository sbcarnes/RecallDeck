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
            
            DrawFlashcard(memDC, &app.card);
            
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
            
            if (app.card.isPressed)
            {
                RECT oldBounds = app.card.bounds;
                
                int newLeft = mousePosition.x - app.card.dragOffset.x;
                
                int newTop = mousePosition.y - app.card.dragOffset.y;
                
                int deltaX = newLeft - app.card.bounds.left;
                
                int deltaY = newTop - app.card.bounds.top;
                
                
                
                /*if (app.card.visibleSide == CARD_FRONT)
                {
                    app.card.visibleSide = CARD_BACK;
                }
                else
                {
                    app.card.visibleSide = CARD_FRONT;
                }*/
                
                if (deltaX != 0 || deltaY != 0)
                {
                    app.card.wasDragged = TRUE;
                }
                OffsetRect(&app.card.bounds, deltaX, deltaY);
                InvalidateRect(hwnd, &oldBounds, FALSE);
                InvalidateRect(hwnd, &app.card.bounds, FALSE);
            }
            else
            {
                BOOL wasHovered = app.card.isHovered;
            
                app.card.isHovered = PtInRect(&app.card.bounds, mousePosition);
                
                if (app.card.isHovered != wasHovered)
                {
                    InvalidateRect(
                        hwnd,
                        &app.card.bounds,
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
            
            if (PtInRect(&app.card.bounds, mousePosition))
            {
                app.card.isPressed = TRUE;
                app.card.wasDragged = FALSE;
                
                app.card.dragOffset.x =
                    mousePosition.x - app.card.bounds.left;
                
                app.card.dragOffset.y =
                    mousePosition.y - app.card.bounds.top;
                    
                SetCapture(hwnd);
                
                InvalidateRect(
                    hwnd,
                    &app.card.bounds,
                    FALSE
                );
            }
        }
        break;
        case WM_LBUTTONUP:
        {
            if (app.card.isPressed)
            {
                POINT mousePosition =
                {
                    GET_X_LPARAM(lParam),
                    GET_Y_LPARAM(lParam)
                };
                
                BOOL isCardClick =
                    !app.card.wasDragged &&
                    PtInRect(&app.card.bounds, mousePosition);
                
                app.card.isPressed = FALSE;
                
                ReleaseCapture();
                
                if (isCardClick)
                {
                    HandleFlashcardClick(
                        &app.card,
                        mousePosition
                    );
                }
                
                app.card.wasDragged = FALSE;
                
                InvalidateRect(
                    hwnd,
                    &app.card.bounds,
                    FALSE
                );
            }
        }
        break;
        case WM_MOUSELEAVE:
        {
            app.trackingMouseLeave = FALSE;
            
            if (app.card.isHovered)
            {
                app.card.isHovered = FALSE;
                
                InvalidateRect(
                    hwnd,
                    &app.card.bounds,
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
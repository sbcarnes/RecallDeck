#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "app.h"

const char g_szClassName[] = "myWindowClass";

HBRUSH g_hbrBackground;
COLORREF g_rgbBackground = RGB(255, 255, 255);

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
            
            DrawFlashcard(hdc, &app.card);
            
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
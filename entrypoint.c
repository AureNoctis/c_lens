#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "utils.c"
#define ARENA_IMPLEMENTATION
#include "arena.h"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "gdi32.lib")


#define global static
#define START_HOTKEY_ID 1
#define CLOSE_HOTKEY_ID 2

global HWND hwnd_overlay;

global i32 screen_width;  
global i32 screen_height;
global i32 screen_x;  
global i32 screen_y;  

global u8* pixels;


HWND create_overlay(HINSTANCE hInstance, const wchar_t* class_name, const wchar_t* overlay_name){

	HWND overlay = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            class_name, overlay_name,
            WS_POPUP,
            screen_x, screen_y,
            screen_width, screen_height,
            NULL, NULL, hInstance, NULL);

    return overlay;
}

LRESULT CALLBACK overlay_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam){
    int return_value = 0;
	switch (msg) {
		case WM_PAINT: {
            printf("wm_paint message\n");
			PAINTSTRUCT ps;
			HDC			hdc = BeginPaint(window, &ps);

			if (pixels != NULL) {
				BITMAPINFO bi = {0};

				bi.bmiHeader.biSize		   = sizeof(BITMAPINFOHEADER);
				bi.bmiHeader.biWidth	   = screen_width;
				bi.bmiHeader.biHeight	   = -screen_height; // Top-down image
				bi.bmiHeader.biPlanes	   = 1;
				bi.bmiHeader.biBitCount	   = 32;
				bi.bmiHeader.biCompression = BI_RGB;

				StretchDIBits(hdc, 0, 0, screen_width, screen_height, // Destination
							  0, 0, screen_width, screen_height,	  // Source
							  pixels, &bi, DIB_RGB_COLORS, SRCCOPY);
			}

			EndPaint(window, &ps);
			return 0;
		}
		case WM_KEYDOWN: {
			if (wparam == VK_ESCAPE) {
				ShowWindow(hwnd_overlay, SW_HIDE);
			}
		} break;

		case WM_CLOSE: {
			DestroyWindow(window);
		} break;

		case WM_DESTROY: {
			UnregisterHotKey(NULL, START_HOTKEY_ID);
			UnregisterHotKey(NULL, CLOSE_HOTKEY_ID);
			hwnd_overlay = NULL;
			PostQuitMessage(0);
		} break;

		default: 
            return DefWindowProcW(window, msg, wparam, lparam);
	}
	return return_value;
}

u32 capture_entire_screen(HDC screen_dc, HDC overlay_dc, HDC memory_dc, HBITMAP bitmap, u8** pixels){
    if (!screen_dc || !overlay_dc || !memory_dc || !bitmap){
        fprintf(stderr, "[ERROR] : Argument(s) is/are invalid\n");
        return 0; 
	}

    HBITMAP old_bitmap = SelectObject(memory_dc, bitmap);
    
    if (!BitBlt(memory_dc, 0, 0, screen_width, screen_height, screen_dc, 0, 0, SRCCOPY | CAPTUREBLT)){
        fprintf(stderr, "[ERROR] : BitBlit failed : %lu\n", GetLastError());
        SelectObject(memory_dc, old_bitmap);
        return 0;
    }
    
    SelectObject(memory_dc, old_bitmap);
	
    BITMAPINFO bi = {0};

	bi.bmiHeader.biSize		   = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth	   = screen_width;
	bi.bmiHeader.biHeight	   = -screen_height;
	bi.bmiHeader.biPlanes	   = 1;
	bi.bmiHeader.biBitCount	   = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	DWORD stride	 = screen_width * 4;
	DWORD image_size = stride * (DWORD)screen_height;

	bi.bmiHeader.biSizeImage = image_size;

    if ((*pixels) == NULL){
        *pixels = malloc(image_size);
    }
    
    u32 lines = GetDIBits(screen_dc, bitmap, 0, (UINT)screen_height, *pixels, &bi, DIB_RGB_COLORS);
    if (lines != screen_height){
        fprintf(stderr, "[ERROR] : GetDIBits failed: %lu\n", GetLastError());
        return 0;
    }
    return 1; 
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    summonConsole();
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    wchar_t const* class_name	= L"ai_overlay_class";
	wchar_t const* overlay_name = L"ai_overlay";

	WNDCLASSW wc     = {0};
    wc.lpfnWndProc   = overlay_callback;
    wc.hInstance     = hInstance;
    wc.lpszClassName = class_name;
    wc.hCursor       = LoadCursorW(NULL, IDC_CROSS);

    if(!RegisterClassW(&wc)){
        MessageBoxW(NULL, L"Failed to register window class", L"ERROR", MB_OK | MB_ICONERROR);
        exit(1);
    }

    if(!RegisterHotKey(NULL, START_HOTKEY_ID, MOD_ALT | MOD_CONTROL | MOD_NOREPEAT, VK_RETURN)){
		MessageBoxW(NULL, L"Failed to register start hotkey", L"ERROR", MB_OK | MB_ICONERROR);
		exit(1);
	}
	if(!RegisterHotKey(NULL, CLOSE_HOTKEY_ID, MOD_CONTROL | MOD_NOREPEAT, 'Q')) {
		MessageBoxW(NULL, L"Failed to register close hotkey", L"ERROR", MB_OK | MB_ICONERROR);
		exit(1);
	}

	screen_width  = GetSystemMetrics(SM_CXSCREEN);
	screen_height = GetSystemMetrics(SM_CYSCREEN);
	screen_x	  = GetSystemMetrics(SM_XVIRTUALSCREEN);
	screen_y	  = GetSystemMetrics(SM_YVIRTUALSCREEN);

	HDC		screen_dc  = GetDC(0);
	HDC		memory_dc  = CreateCompatibleDC(screen_dc);
	HBITMAP bitmap	   = CreateCompatibleBitmap(screen_dc, screen_width, screen_height);

	HDC overlay_dc;

	MSG msg = {0};
    while(GetMessageW(&msg, NULL, 0, 0)){
        
        if(msg.message == WM_HOTKEY){
            if(msg.wParam == START_HOTKEY_ID){

                if(hwnd_overlay == NULL){
                    hwnd_overlay = create_overlay(hInstance, class_name, overlay_name);
                    SetLayeredWindowAttributes(hwnd_overlay, 0, 255, LWA_ALPHA);
                    overlay_dc = GetDC(hwnd_overlay);
                }
				capture_entire_screen(screen_dc, overlay_dc, memory_dc, bitmap, &pixels);
                if(!pixels){
                    fprintf(stderr, "[ERROR] : Unable to capture screen\n");
                    exit(1);
                }

				ShowWindow(hwnd_overlay, SW_SHOW);
            }
            else if(msg.wParam == CLOSE_HOTKEY_ID){
                if(hwnd_overlay != NULL)
    				PostMessage(hwnd_overlay, WM_CLOSE, 0, 0);
                else
                    PostQuitMessage(0); // if overlay creation failed 
			}
        }else{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

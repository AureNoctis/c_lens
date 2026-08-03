#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "utils.c"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "gdi32.lib")


#define global static
#define START_HOTKEY_ID 1

global HWND hwnd_overlay;


HWND create_overlay(HINSTANCE hInstance, const wchar_t* class_name, const wchar_t* overlay_name){
	int screen_width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int screen_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	int screen_x	  = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screen_y	  = GetSystemMetrics(SM_YVIRTUALSCREEN);

    HWND hwnd_overlay = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            class_name, overlay_name,
            WS_POPUP | WS_VISIBLE,
            screen_x, screen_y,
            screen_width, screen_height,
            NULL, NULL, hInstance, NULL);

    return hwnd_overlay;
}

LRESULT CALLBACK overlay_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam){
    switch(msg){
    case WM_KEYDOWN: 
        {
            if(wparam == VK_ESCAPE){
                ShowWindow(hwnd_overlay, SW_HIDE);
            }
            return 0;
        }
        break;
    default:
        return DefWindowProcW(window, msg, wparam, lparam);
    }
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    summonConsole();


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
		MessageBoxW(NULL, L"Failed to register hotkey", L"ERROR", MB_OK | MB_ICONERROR);
		exit(1);
	}

    MSG msg = {0};
    while(GetMessageW(&msg, NULL, 0, 0)){
        
        if(msg.message == WM_HOTKEY){
            if(msg.wParam == START_HOTKEY_ID){
                if(hwnd_overlay == NULL){
                    hwnd_overlay = create_overlay(hInstance, class_name, overlay_name);
                    SetLayeredWindowAttributes(hwnd_overlay, 0, 100, LWA_ALPHA);
                }
                ShowWindow(hwnd_overlay, SW_SHOW);

            }
        }else{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }



    UnregisterHotKey(NULL, START_HOTKEY_ID);
    return 0;
}

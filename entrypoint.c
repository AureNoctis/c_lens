#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// #include <stdio.h>
#include <stdlib.h>

// #include "utils.c"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "gdi32.lib")


#define global static
#define START_HOTKEY_ID 1
#define CLOSE_HOTKEY_ID 2

global HWND		  hwnd_overlay;
global HBITMAP	  hbm_screen	  = NULL;
global void*	  raw_pixle_bytes = NULL;
global BITMAPINFO bmi_info		  = {0};

HWND create_overlay(HINSTANCE hInstance, const wchar_t* class_name, const wchar_t* overlay_name){
	int screen_width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int screen_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	int screen_x	  = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screen_y	  = GetSystemMetrics(SM_YVIRTUALSCREEN);

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
    switch(msg){
    case WM_KEYDOWN: 
        {
            if(wparam == VK_ESCAPE){
                ShowWindow(hwnd_overlay, SW_HIDE);
            }
        }
        break;
    case WM_CLOSE:
        {
            DestroyWindow(window);
        }
        break;
    case WM_DESTROY:
        {
            UnregisterHotKey(NULL, START_HOTKEY_ID);
			UnregisterHotKey(NULL, CLOSE_HOTKEY_ID);
            hwnd_overlay = NULL;
            PostQuitMessage(0);
		}
        break;
    default:
        return DefWindowProcW(window, msg, wparam, lparam);
    }
    return return_value;
}

void capture_entire_screen(HWND overlay_window){
	int main_window_width  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int main_window_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	int x				   = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int y				   = GetSystemMetrics(SM_YVIRTUALSCREEN);

    ShowWindow(overlay_window, SW_HIDE);
	Sleep(30);

    HDC main_window_dc = GetDC(NULL);

    bmi_info.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi_info.bmiHeader.biWidth		= main_window_width;
	bmi_info.bmiHeader.biHeight		= -main_window_height; 
	bmi_info.bmiHeader.biPlanes		= 1;
	bmi_info.bmiHeader.biBitCount	= 32;
	bmi_info.bmiHeader.biCompression = BI_RGB;

    if(hbm_screen){
        DeleteObject(hbm_screen);
    }
    hbm_screen = CreateDIBSection(main_window_dc, &bmi_info, DIB_RGB_COLORS, &raw_pixle_bytes, NULL, 0);
    HGDIOBJ old_bmp = SelectObject(main_window_dc, hbm_screen);


    SelectObject(main_window_dc, old_bmp);
    DeleteDC(main_window_dc);

    ShowWindow(overlay_window, SW_SHOW);
    SetForegroundWindow(overlay_window);
    SetFocus(overlay_window);
    InvalidateRect(overlay_window, NULL, TRUE);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;

    // summonConsole();
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

	MSG msg = {0};
    while(GetMessageW(&msg, NULL, 0, 0)){
        
        if(msg.message == WM_HOTKEY){
            if(msg.wParam == START_HOTKEY_ID){
                if(hwnd_overlay == NULL){
                    hwnd_overlay = create_overlay(hInstance, class_name, overlay_name);
                    SetLayeredWindowAttributes(hwnd_overlay, 0, 100, LWA_ALPHA);
                }
				// capture_entire_screen(hwnd_overlay);
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

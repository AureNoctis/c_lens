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

// ==============================================
//                   structs
// ==============================================

typedef struct {
	i32 width;
	i32 height;
	i32 x;
	i32 y;
} Screen_dimension;

// WAD : Window Associated Data
typedef struct {
	Screen_dimension  screen;
	u8*				  pixels;
	HWND			  overlay;
	wchar_t const*	  overlay_class_name;
	wchar_t const*	  overlay_name;
    BITMAPINFO        bitmap_info;
} WAD;

// ==============================================
//                   globals
// ==============================================





// ==============================================
//                  functions
// ==============================================

WAD* init_window_associated_data(Arena* arena, const wchar_t* overlay_class_name, const wchar_t* overlay_name){

	WAD* wad = (WAD*)arena_push_type(arena, WAD);

	wad->screen.width	= GetSystemMetrics(SM_CXSCREEN);
	wad->screen.height	= GetSystemMetrics(SM_CYSCREEN);
	wad->screen.x		= GetSystemMetrics(SM_XVIRTUALSCREEN);
	wad->screen.y		= GetSystemMetrics(SM_YVIRTUALSCREEN);

	wad->overlay_class_name = overlay_class_name;
	wad->overlay_name		= overlay_name;

	wad->bitmap_info.bmiHeader.biSize		 = sizeof(BITMAPINFOHEADER);
	wad->bitmap_info.bmiHeader.biWidth		 = wad->screen.width;
	wad->bitmap_info.bmiHeader.biHeight		 = -wad->screen.height; // Top-down image
	wad->bitmap_info.bmiHeader.biPlanes		 = 1;
	wad->bitmap_info.bmiHeader.biBitCount	 = 32;
	wad->bitmap_info.bmiHeader.biCompression = BI_RGB;

	DWORD stride						   = wad->screen.width * 4;
	DWORD image_size					   = stride * (DWORD)wad->screen.height;
	wad->bitmap_info.bmiHeader.biSizeImage = image_size;

	return wad;
}


HWND create_overlay(HINSTANCE hInstance, WAD* wad){

	HWND overlay = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            wad->overlay_class_name, wad->overlay_name,
            WS_POPUP,
            wad->screen.x, wad->screen.y,
            wad->screen.width, wad->screen.height,
            NULL, NULL, hInstance, NULL);

    return overlay;
}

LRESULT CALLBACK overlay_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam){

	WAD* wad = (WAD*)GetWindowLongPtrW(window, GWLP_USERDATA);

	switch (msg) {
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC			hdc = BeginPaint(window, &ps);

			if (wad->pixels != NULL) {

				StretchDIBits(hdc, 0, 0, wad->screen.width, wad->screen.height, // Destination
							  0, 0, wad->screen.width, wad->screen.height,	  // Source
							  wad->pixels, &wad->bitmap_info, DIB_RGB_COLORS, SRCCOPY);
			}

			EndPaint(window, &ps);
			return 0;
		}
		case WM_KEYDOWN: {
			if (wparam == VK_ESCAPE) {
				ShowWindow(wad->overlay, SW_HIDE);
			}
            return 0;
		} break;

		case WM_CLOSE: {
			DestroyWindow(window);
            return 0;
		} break;

		case WM_DESTROY: {
			UnregisterHotKey(NULL, START_HOTKEY_ID);
			UnregisterHotKey(NULL, CLOSE_HOTKEY_ID);
			PostQuitMessage(0);
            return 0;
		} break;

		default: 
            return DefWindowProcW(window, msg, wparam, lparam);
	}
}

u32 capture_entire_screen(WAD* wad, HDC screen_dc, HDC overlay_dc, HDC memory_dc, HBITMAP bitmap, u8** pixels){
    if (!screen_dc || !overlay_dc || !memory_dc || !bitmap){
        fprintf(stderr, "[ERROR] : Argument(s) is/are invalid\n");
        return 0; 
	}

    HBITMAP old_bitmap = SelectObject(memory_dc, bitmap);
    
    if (!BitBlt(memory_dc, 0, 0, wad->screen.width, wad->screen.height, screen_dc, 0, 0, SRCCOPY | CAPTUREBLT)){
        fprintf(stderr, "[ERROR] : BitBlit failed : %lu\n", GetLastError());
        SelectObject(memory_dc, old_bitmap);
        return 0;
    }
    
    SelectObject(memory_dc, old_bitmap);
	
    if ((*pixels) == NULL){
        *pixels = malloc(wad->bitmap_info.bmiHeader.biSizeImage);
    }
    
    u32 lines = GetDIBits(screen_dc, bitmap, 0, (UINT)wad->screen.height, *pixels, &wad->bitmap_info, DIB_RGB_COLORS);
    if (lines != wad->screen.height){
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


    Arena* arena = arena_alloc(GB(1));
	WAD*   wad	 = (WAD*)init_window_associated_data(arena, L"ai_overlay_class", L"ai_overlay");

	WNDCLASSW wc     = {0};
    wc.lpfnWndProc   = overlay_callback;
    wc.hInstance     = hInstance;
    wc.lpszClassName = wad->overlay_class_name;
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


	HDC		screen_dc  = GetDC(0);
	HDC		memory_dc  = CreateCompatibleDC(screen_dc);
    HBITMAP bitmap	   = CreateCompatibleBitmap(screen_dc, wad->screen.width, wad->screen.height);

	HDC overlay_dc;


	wad->overlay = create_overlay(hInstance, wad);
	SetWindowLongPtrW(wad->overlay, GWLP_USERDATA, (LONG_PTR)wad);
	SetLayeredWindowAttributes(wad->overlay, 0, 255, LWA_ALPHA);
	overlay_dc = GetDC(wad->overlay);




	MSG msg = {0};
    while(GetMessageW(&msg, NULL, 0, 0)){
        
        if(msg.message == WM_HOTKEY){
            if(msg.wParam == START_HOTKEY_ID){

				capture_entire_screen(wad, screen_dc, overlay_dc, memory_dc, bitmap, &wad->pixels);
                if(!wad->pixels){
                    fprintf(stderr, "[ERROR] : Unable to capture screen\n");
                    exit(1);
                }

				ShowWindow(wad->overlay, SW_SHOW);
            }
            else if(msg.wParam == CLOSE_HOTKEY_ID){
                if(wad->overlay != NULL)
    				PostMessage(wad->overlay, WM_CLOSE, 0, 0);
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

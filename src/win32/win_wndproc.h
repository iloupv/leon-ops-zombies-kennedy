#pragma once

#include <Windows.h>

struct WinVars_t // sizeof=0x20
{                                       // XREF: .data:WinVars_t g_wv/r
    HINSTANCE__ *reflib_library;
    int reflib_active;
    HWND__ *hWnd;                       // XREF: IN_RecenterMouse(void)+A/r
                                        // IN_IsForegroundWindow(void)+B/r ...
    HINSTANCE__ *hInstance;             // XREF: WinMain(x,x,x,x)+13F/w
                                        // Win_RegisterClass+24/r ...
    int activeApp;                      // XREF: Sys_UpdateHotkeyBlock(void)+8/r
                                        // MainWndProc(HWND__ *,uint,uint,long)+3C2/r ...
    int isMinimized;                    // XREF: WinMain(x,x,x,x):loc_7DB70E/r
                                        // VID_AppActivate+9/w ...
    int recenterMouse;                  // XREF: IN_MouseMove+FF/w
                                        // IN_MouseMove+104/r
    unsigned int sysMsgTime;            // XREF: Sys_SetBlockSystemHotkeys(int)+4/r
};

void __cdecl Sys_SetBlockSystemHotkeys(int block);
LRESULT __stdcall LowLevelKeyboardProc(int nCode, WPARAM wParam, unsigned int *lParam);
bool __cdecl Sys_AllowVidRestart();
void __cdecl Sys_UpdateHotkeyBlock();
int __stdcall MainWndProc(HWND__ *hWnd, UINT uMsg, signed int wParam, LPARAM lParam);
void __cdecl VID_AppActivate(unsigned int activeState, int minimize);
unsigned __int8 __cdecl MapKey(int key, unsigned int wParam);
unsigned int __cdecl AdustKeyForNumericKeypad(unsigned int key, unsigned int wParam, unsigned int extended);
bool __cdecl IsNumLockAffectedVK(unsigned int wParam);

extern WinVars_t g_wv;

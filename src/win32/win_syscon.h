#pragma once

#include <Windows.h>

struct WinConData // sizeof=0x620
{                                       // XREF: .data:s_wcd/r
    HWND__ *hWnd;                       // XREF: InputLineWndProc(HWND__ *,uint,uint,long)+3A/r
                                        // Sys_CreateConsole(HINSTANCE__ *)+223/w ...
    HWND__ *hwndBuffer;                 // XREF: Sys_CreateConsole(HINSTANCE__ *)+386/w
                                        // Sys_CreateConsole(HINSTANCE__ *)+396/r ...
    HWND__ *codLogo;                    // XREF: Sys_CreateConsole(HINSTANCE__ *)+300/w
                                        // Sys_CreateConsole(HINSTANCE__ *)+313/r
    HFONT__ *hfBufferFont;              // XREF: Sys_CreateConsole(HINSTANCE__ *)+298/w
                                        // Sys_CreateConsole(HINSTANCE__ *)+38D/r ...
    HWND__ *hwndInputLine;              // XREF: InputLineWndProc(HWND__ *,uint,uint,long)+6E/r
                                        // InputLineWndProc(HWND__ *,uint,uint,long)+129/r ...
    char errorString[512];              // XREF: Sys_SetErrorText(char const *)+C/o
    char consoleText[512];              // XREF: InputLineWndProc(HWND__ *,uint,uint,long)+7B/o
                                        // InputLineWndProc(HWND__ *,uint,uint,long)+DA/o ...
    char returnedText[512];             // XREF: Sys_ConsoleInput(void)+1C/o
                                        // Sys_ConsoleInput(void)+58/o
    int windowWidth;                    // XREF: Sys_CreateConsole(HINSTANCE__ *)+1A0/w
                                        // ConWndProc+ED/w
    int windowHeight;                   // XREF: Sys_CreateConsole(HINSTANCE__ *)+1B5/w
                                        // ConWndProc+F6/w
    int (__stdcall *SysInputLineWndProc)(HWND__ *, unsigned int, unsigned int, int);
                                        // XREF: InputLineWndProc(HWND__ *,uint,uint,long)+178/r
                                        // Sys_CreateConsole(HINSTANCE__ *)+3B6/w
};

LRESULT __stdcall InputLineWndProc(HWND__ *hWnd, UINT uMsg, HWND__ *wParam, LPARAM lParam);
void __cdecl Sys_CreateConsole(HINSTANCE__ *hInstance);
LRESULT __stdcall ConWndProc(HWND__ *hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
char *__cdecl Conbuf_CleanText(const char *source, char *target, int sizeofTarget);
void __cdecl Sys_DestroyConsole();
void __cdecl Sys_ShowConsole();
char *__cdecl Sys_ConsoleInput();
void __cdecl Conbuf_AppendText(char *pMsg);
void __cdecl Conbuf_AppendTextInMainThread(char *msg);
void __cdecl Sys_SetErrorText(const char *buf);

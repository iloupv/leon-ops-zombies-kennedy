#include "win_syscon.h"
#include <universal/q_shared.h>
#include "win_main.h"
#include <client/cl_console.h>
#include <qcommon/threads.h>

WinConData s_wcd;
unsigned int s_totalChars;

LRESULT __stdcall InputLineWndProc(HWND__ *hWnd, UINT uMsg, HWND__ *wParam, LPARAM lParam)
{
    char displayBuffer[1024]; // [esp+20h] [ebp-808h] BYREF
    char inputBuffer[1028]; // [esp+420h] [ebp-408h] BYREF

    if ( uMsg == 8 )
    {
        if ( wParam == s_wcd.hWnd )
        {
            SetFocus(hWnd);
            return 0;
        }
    }
    else if ( uMsg == 258 && wParam == (HWND__ *)13 )
    {
        GetWindowTextA(s_wcd.hwndInputLine, inputBuffer, 1024);
        strncat(s_wcd.consoleText, inputBuffer, 512 - strlen(s_wcd.consoleText) - 5);
        strcat(s_wcd.consoleText, "\n");
        SetWindowTextA(s_wcd.hwndInputLine, "");
        Com_sprintf(displayBuffer, 0x400u, "]%s\n", inputBuffer);
        Sys_Print(displayBuffer);
        return 0;
    }
    return CallWindowProcA((WNDPROC)s_wcd.SysInputLineWndProc, hWnd, uMsg, (WPARAM)wParam, lParam);
}

void __cdecl Sys_CreateConsole(HINSTANCE__ *hInstance)
{
    HWND DesktopWindow; // eax
    HWND v2; // eax
    int v3; // eax
    HDC v4; // [esp-4h] [ebp-8064h]
    HANDLE lParam; // [esp+0h] [ebp-8060h]
    WNDCLASSA WndClass; // [esp+4h] [ebp-805Ch] BYREF
    int v7; // [esp+2Ch] [ebp-8034h]
    char target[16384]; // [esp+30h] [ebp-8030h] BYREF
    LPCSTR lpClassName; // [esp+4030h] [ebp-4030h]
    LPCSTR lpWindowName; // [esp+4034h] [ebp-402Ch]
    int cHeight; // [esp+4038h] [ebp-4028h]
    HDC hdc; // [esp+403Ch] [ebp-4024h]
    tagRECT Rect; // [esp+4040h] [ebp-4020h] BYREF
    int DeviceCaps; // [esp+4050h] [ebp-4010h]
    unsigned int dwStyle; // [esp+4054h] [ebp-400Ch]
    char text[16388]; // [esp+4058h] [ebp-4008h] BYREF

    dwStyle = -2134245376;
    lpClassName = "CoD Black Ops WinConsole";
    lpWindowName = "CoD Black Ops Console";
    WndClass.style = 0;
    WndClass.lpfnWndProc = ConWndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hInstance;
    WndClass.hIcon = LoadIconA(hInstance, (LPCSTR)1);
    WndClass.hCursor = LoadCursorA(0, (LPCSTR)0x7F00);
    WndClass.hbrBackground = (HBRUSH__ *)5;
    WndClass.lpszMenuName = 0;
    WndClass.lpszClassName = "CoD Black Ops WinConsole";
    if ( RegisterClassA(&WndClass) )
    {
        Rect.left = 0;
        Rect.right = 620;
        Rect.top = 0;
        Rect.bottom = 450;
        AdjustWindowRect(&Rect, dwStyle, 0);
        DesktopWindow = GetDesktopWindow();
        hdc = GetDC(DesktopWindow);
        DeviceCaps = GetDeviceCaps(hdc, 8);
        v7 = GetDeviceCaps(hdc, 10);
        v4 = hdc;
        v2 = GetDesktopWindow();
        ReleaseDC(v2, v4);
        s_wcd.windowWidth = Rect.right - Rect.left + 1;
        s_wcd.windowHeight = Rect.bottom - Rect.top + 1;
        s_wcd.hWnd = CreateWindowExA(
                                     0,
                                     lpClassName,
                                     lpWindowName,
                                     dwStyle,
                                     (DeviceCaps - 600) / 2,
                                     (v7 - 450) / 2,
                                     Rect.right - Rect.left + 1,
                                     Rect.bottom - Rect.top + 1,
                                     0,
                                     0,
                                     hInstance,
                                     0);
        if ( s_wcd.hWnd )
        {
            hdc = GetDC(s_wcd.hWnd);
            v3 = GetDeviceCaps(hdc, 90);
            cHeight = -MulDiv(8, v3, 72);
            s_wcd.hfBufferFont = CreateFontA(cHeight, 0, 0, 0, 300, 0, 0, 0, 1u, 0, 0, 0, 0x31u, "Courier New");
            ReleaseDC(s_wcd.hWnd, hdc);
            lParam = LoadImageA(0, "codlogo.bmp", 0, 0, 0, 0x10u);
            if ( lParam )
            {
                s_wcd.codLogo = CreateWindowExA(0, "Static", 0, 0x5000000Eu, 5, 5, 0, 0, s_wcd.hWnd, (HMENU)1, hInstance, 0);
                SendMessageA(s_wcd.codLogo, 0x172u, 0, (LPARAM)lParam);
            }
            s_wcd.hwndInputLine = CreateWindowExA(
                                                            0,
                                                            "edit",
                                                            0,
                                                            0x50800080u,
                                                            6,
                                                            400,
                                                            608,
                                                            20,
                                                            s_wcd.hWnd,
                                                            (HMENU)0x65,
                                                            hInstance,
                                                            0);
            s_wcd.hwndBuffer = CreateWindowExA(
                                                     0,
                                                     "edit",
                                                     0,
                                                     0x50A00844u,
                                                     6,
                                                     70,
                                                     606,
                                                     324,
                                                     s_wcd.hWnd,
                                                     (HMENU)0x64,
                                                     hInstance,
                                                     0);
            SendMessageA(s_wcd.hwndBuffer, 0x30u, (WPARAM)s_wcd.hfBufferFont, 0);
            s_wcd.SysInputLineWndProc = (int (__stdcall *)(HWND__ *, unsigned int, unsigned int, int))SetWindowLongA(
                                                                                                                                                                                                    s_wcd.hwndInputLine,
                                                                                                                                                                                                    -4,
                                                                                                                                                                                                    (int)InputLineWndProc);
            SendMessageA(s_wcd.hwndInputLine, 0x30u, (WPARAM)s_wcd.hfBufferFont, 0);
            SetFocus(s_wcd.hwndInputLine);
            Con_GetTextCopy(text, 0x4000);
            Conbuf_CleanText(text, target, 0x4000);
            SetWindowTextA(s_wcd.hwndBuffer, target);
        }
    }
}

LRESULT __stdcall ConWndProc(HWND__ *hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char *cmdString; // [esp+18h] [ebp-20h]

    switch ( uMsg )
    {
        case WM_SIZE:
            SetWindowPos(s_wcd.hwndBuffer, 0, 5, 70, (unsigned __int16)lParam - 15, HIWORD(lParam) - 100, 0);
            SetWindowPos(s_wcd.hwndInputLine, 0, 5, HIWORD(lParam) - 100 + 78, (unsigned __int16)lParam - 15, 20, 0);
            s_wcd.windowWidth = (unsigned __int16)lParam;
            s_wcd.windowHeight = HIWORD(lParam);
            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
        case WM_ACTIVATE:
            if ( (_WORD)wParam )
                SetFocus(s_wcd.hwndInputLine);
            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
        case WM_CLOSE:
            if (IsDedicatedServer())
            {
                cmdString = (char *)Com_AllocEvent(5);
                strcpy(cmdString, "quit");
                Sys_QueEvent(0, SE_CONSOLE, 0, 0, strlen(cmdString) + 1, cmdString);
            }
            else
            {
                PostQuitMessage(0);
            }
            return 0;
        default:
            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
}

char *__cdecl Conbuf_CleanText(const char *source, char *target, int sizeofTarget)
{
    const char *start; // [esp+0h] [ebp-8h]
    const char *last; // [esp+4h] [ebp-4h]

    start = target;
    last = &target[sizeofTarget - 3];
    while ( *source && target <= last )
    {
        if ( *source == 10 && source[1] == 13 )
        {
            *target = 13;
            target[1] = 10;
            target += 2;
            source += 2;
        }
        else if ( *source == 13 )
        {
            *target = 13;
            target[1] = 10;
            target += 2;
            ++source;
        }
        else if ( *source == 10 )
        {
            *target = 13;
            target[1] = 10;
            target += 2;
            ++source;
        }
        else if ( source && *source == 94 && source[1] && source[1] != 94 && source[1] >= 48 && source[1] <= 64 )
        {
            source += 2;
        }
        else
        {
            *target++ = *source++;
        }
    }
    *target = 0;
    return (char *)(target - start);
}

void __cdecl Sys_DestroyConsole()
{
    if ( s_wcd.hWnd )
    {
        ShowWindow(s_wcd.hWnd, 0);
        CloseWindow(s_wcd.hWnd);
        DestroyWindow(s_wcd.hWnd);
        s_wcd.hWnd = 0;
    }
}

void __cdecl Sys_ShowConsole()
{
    HMODULE ModuleHandleA; // eax

    if ( Sys_IsHeadless() ) // zombies: headless never creates or shows the console window
        return;
    if ( !s_wcd.hWnd )
    {
        ModuleHandleA = GetModuleHandleA(0);
        Sys_CreateConsole(ModuleHandleA);
        if ( !s_wcd.hWnd
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_syscon.cpp", 379, 0, "%s", "s_wcd.hWnd") )
        {
            __debugbreak();
        }
    }
    ShowWindow(s_wcd.hWnd, 1);
    SendMessageA(s_wcd.hwndBuffer, 0xB6u, 0, 0xFFFF);
}

char *__cdecl Sys_ConsoleInput()
{
    char v1; // dl
    char *returnedText; // [esp+8h] [ebp-8h]
    char *consoleText; // [esp+Ch] [ebp-4h]

    if ( !s_wcd.consoleText[0] )
        return 0;
    consoleText = s_wcd.consoleText;
    returnedText = s_wcd.returnedText;
    do
    {
        v1 = *consoleText;
        *returnedText++ = *consoleText++;
    }
    while ( v1 );
    s_wcd.consoleText[0] = 0;
    return s_wcd.returnedText;
}

void __cdecl Conbuf_AppendText(char *pMsg)
{
    char target[32772]; // [esp+20h] [ebp-8010h] BYREF
    char *source; // [esp+8028h] [ebp-8h]
    char *v3; // [esp+802Ch] [ebp-4h]

    if ( !s_wcd.hwndBuffer
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_syscon.cpp", 414, 0, "%s", "s_wcd.hwndBuffer") )
    {
        __debugbreak();
    }
    if ( strlen(pMsg) <= 0x3FFF )
        source = pMsg;
    else
        source = &pMsg[strlen(pMsg) - 0x3FFF];
    v3 = Conbuf_CleanText(source, target, 0x8000);
    s_totalChars += (unsigned int)v3;
    if ( s_totalChars <= 0x4000 )
    {
        SendMessageA(s_wcd.hwndBuffer, 0xB1u, 0xFFFFu, 0xFFFF);
    }
    else
    {
        SendMessageA(s_wcd.hwndBuffer, 0xB1u, 0, -1);
        s_totalChars = (unsigned int)v3;
    }
    SendMessageA(s_wcd.hwndBuffer, 0xB6u, 0, 0xFFFF);
    SendMessageA(s_wcd.hwndBuffer, 0xB7u, 0, 0);
    SendMessageA(s_wcd.hwndBuffer, 0xC2u, 0, (LPARAM)target);
}

void __cdecl Conbuf_AppendTextInMainThread(char *msg)
{
    if ( s_wcd.hwndBuffer )
    {
        if ( Sys_IsMainThread() )
            Conbuf_AppendText(msg);
    }
}

void __cdecl Sys_SetErrorText(const char *buf)
{
    HWND ActiveWindow; // eax

    I_strncpyz(s_wcd.errorString, buf, 512);
    if ( Sys_IsHeadless() ) // zombies: no message box (Sys_Error ends headless runs before this anyway)
        return;
    DestroyWindow(s_wcd.hwndInputLine);
    s_wcd.hwndInputLine = 0;
    ActiveWindow = GetActiveWindow();
    MessageBoxA(ActiveWindow, buf, "Error", 0x10u);
}


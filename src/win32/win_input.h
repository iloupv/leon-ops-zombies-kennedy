#pragma once

#include <Windows.h>

struct __declspec(align(4)) WinMouseVars_t // sizeof=0x10
{                                       // XREF: .data:s_wmv/r
    int oldButtonState;                 // XREF: IN_MouseEvent(int)+16/r
                                        // IN_MouseEvent(int)+89/w
    tagPOINT oldPos;                    // XREF: IN_SetCursorPos(int,int)+33/w
                                        // IN_SetCursorPos(int,int)+3B/w ...
    bool mouseActive;                   // XREF: IN_ActivateMouse(int)+4C/w
                                        // IN_ActivateMouse(int)+5B/r ...
    bool mouseInitialized;              // XREF: IN_ActivateMouse(int)+3/r
                                        // IN_DeactivateMouse(void)+3/r ...
    // padding byte
    // padding byte
};

void __cdecl IN_StartupGamepads();
void __cdecl IN_RecenterMouse();
bool __cdecl IN_IsForegroundWindow();
void __cdecl IN_ActivateMouse(int force);
void __cdecl IN_DeactivateMouse();
void IN_DeactivateWin32Mouse();
void __cdecl IN_StartupMouse();
void __cdecl IN_MouseEvent(int mstate);
void __cdecl IN_SetCursorPos(unsigned int x, unsigned int y);
void __cdecl IN_ShowSystemCursor(bool show);
void __cdecl IN_Startup();
void __cdecl IN_Shutdown();
void __cdecl IN_Init();
void __cdecl IN_Activate(int active);
void __cdecl IN_Frame();
void IN_GamepadsMove();
void IN_MouseMove();
void __cdecl IN_ClampMouseMove(tagPOINT *curPos);

#pragma once

struct keyBoardUtitlity // sizeof=0x8
{                                       // XREF: .data:keyBoardUtitlity uiKeyboard/r
    bool keyboardActive;                // XREF: UI_SetActiveKeyboardType(int):loc_7443D0/r
                                        // UI_SetActiveKeyboardType(int)+75/w ...
    // padding byte
    // padding byte
    // padding byte
    int keyboardType;                   // XREF: UI_GetActiveKeyboardType(void)+3/r
};

int __cdecl UI_GetActiveKeyboardType();
void __cdecl UI_SetActiveKeyboardType(int keyboardType);
void __cdecl UI_ClearKeyboardActive();
void __cdecl UI_Keyboard_New_f();
void __cdecl UI_KeyboardComplete(int localClientNum);
void __cdecl UI_Keyboard_Complete_f();
void __cdecl UI_Clear_Keyboard_Active_f();

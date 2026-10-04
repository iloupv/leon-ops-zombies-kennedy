#include "ui_keyboard.h"
#include <universal/assertive.h>
#include <qcommon/cmd.h>
#include <demo/demo_ui.h>
#include "ui_utils.h"
#include <qcommon/com_clients.h>
#include <qcommon/profanityfilter.h>
#include <stringed/stringed_hooks.h>

keyBoardUtitlity uiKeyboard;

int __cdecl UI_GetActiveKeyboardType()
{
    return uiKeyboard.keyboardType;
}

void __cdecl UI_SetActiveKeyboardType(int keyboardType)
{
    if ( uiKeyboard.keyboardType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_keyboard.cpp",
                    76,
                    0,
                    "%s",
                    "uiKeyboard.keyboardType == KEYBOARD_TYPE_NONE") )
    {
        __debugbreak();
    }
    if ( uiKeyboard.keyboardActive
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui\\ui_keyboard.cpp",
                    77,
                    0,
                    "%s",
                    "uiKeyboard.keyboardActive == false") )
    {
        __debugbreak();
    }
    uiKeyboard.keyboardType = keyboardType;
    uiKeyboard.keyboardActive = 1;
}

void __cdecl UI_ClearKeyboardActive()
{
    uiKeyboard.keyboardActive = 0;
    uiKeyboard.keyboardType = 0;
}

void __cdecl UI_Keyboard_New_f()
{
    const char *v0; // eax
    int keyboardType; // [esp+4Ch] [ebp-10h]
    char *defaultString; // [esp+54h] [ebp-8h]
    char *title; // [esp+58h] [ebp-4h]

    title = 0;
    defaultString = 0;
    if ( Cmd_Argc() == 2 || Cmd_Argc() == 5 )
    {
        v0 = Cmd_Argv(1);
        keyboardType = atoi(v0);
        if ( keyboardType > 0 && keyboardType < 3 )
        {
            if ( keyboardType == 1 )
            {
                title = (char *)Demo_GetKeyboardTitle();
                defaultString = (char *)Demo_GetKeyboardDefaultText();
                Demo_GetKeyboardTextSize();
            }
            strlen(SEH_LocalizeTextMessage(title, "menu prompt", LOCMSG_SAFE));
            UI_SetActiveKeyboardType(keyboardType);
            Dvar_SetStringByName("ui_keyboardtitle", title);
            Dvar_SetStringByName("ui_keyboard_dvar_edit", defaultString);
            UI_OpenMenu(0, "popmenu_ui_keyboard_text");
        }
        else
        {
            Com_Printf(16, "Unsupported keyboard type %d.\n", keyboardType);
        }
    }
    else
    {
        Com_Printf(16, "Usage: ui_keyboard_new <type>\n OR \n ui_keyboard_new <type> <title> <defaultString> <size>.\n");
    }
}

void __cdecl UI_KeyboardComplete(int localClientNum)
{
    const char *String; // eax
    int ControllerIndex; // eax
    const char *errorString; // [esp+4h] [ebp-414h]
    char textReturned[1028]; // [esp+8h] [ebp-410h] BYREF
    bool errorReported; // [esp+413h] [ebp-5h]
    int localControllerIndex; // [esp+414h] [ebp-4h]

    errorReported = 0;
    errorString = (char *)"";
    String = Dvar_GetString("ui_keyboard_dvar_new");
    Com_sprintf(textReturned, 0x400u, "%s", String);
    if ( UI_VerifyString(textReturned) )
    {
        if ( textReturned[0] )
        {
            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            if ( isBadWord(ControllerIndex, textReturned) )
            {
                errorReported = 1;
                errorString = "MENU_OFFENSIVETEXT";
            }
        }
    }
    else
    {
        errorReported = 1;
        errorString = "MENU_INVALIDCHARS";
    }
    if ( !errorReported || UI_GetActiveKeyboardType() == 1 )
    {
        localControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        if ( UI_GetActiveKeyboardType() == 1 )
            Demo_KeyboardComplete(localClientNum, textReturned, errorReported, (char*)errorString);
        UI_ClearKeyboardActive();
    }
    else
    {
        Com_SetErrorMessage((char *)errorString);
        UI_OpenMenu(localClientNum, "error_popmenu_party");
        UI_ClearKeyboardActive();
    }
}

void __cdecl UI_Keyboard_Complete_f()
{
    UI_KeyboardComplete(0);
}

void __cdecl UI_Clear_Keyboard_Active_f()
{
    UI_ClearKeyboardActive();
}


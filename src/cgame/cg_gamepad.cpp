#include "cg_gamepad.h"
#include <cgame_mp/cg_local_mp.h>
#include <client_mp/cl_input_mp.h>
#include "cg_playerstate.h"
#include "cg_main.h"
#include <cgame_mp/cg_main_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <demo/demo_playback.h>
#include <client/cl_keys.h>
#include <ui/ui_shared.h>

void __cdecl CG_HandleSpecialStateInput(int localClientNum, bitarray<51> *button_bits)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.pm_type == 4 || (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 )
        CG_HandleSpectatorInput(localClientNum, button_bits);
}

void __cdecl CG_HandleSpectatorInput(int localClientNum, bitarray<51> *button_bits)
{
    bool thirdPerson; // [esp+Bh] [ebp-Dh]
    const cg_s *cgameGlob; // [esp+Ch] [ebp-Ch]
    bool nextSpecMode; // [esp+13h] [ebp-5h]
    int ps_otherFlags; // [esp+14h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.pm_type != 4
        && (cgameGlob->predictedPlayerState.otherFlags & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_gamepad.cpp",
                    27,
                    0,
                    "%s",
                    "(cgameGlob->predictedPlayerState.pm_type == PM_SPECTATOR) || (cgameGlob->predictedPlayerState.otherFlags & POF_FOLLOW)") )
    {
        __debugbreak();
    }
    ps_otherFlags = cgameGlob->predictedPlayerState.otherFlags;
    nextSpecMode = CL_KeyActiveOrPressed(localClientNum, 0xEu);
    if ( nextSpecMode )
        CG_ResetSpectatorViewAngles(localClientNum);
    if ( (ps_otherFlags & 0x10) != 0 )
    {
        if ( nextSpecMode && cgameGlob->renderingThirdPerson == TP_FOR_MODEL )
        {
            CG_SetThirdPerson(0);
            CL_UpdateCmdButton(localClientNum, button_bits, 14, 0, 1);
            Dvar_SetBool((dvar_s *)cg_spectateThirdPerson, 0);
        }
        else
        {
            CL_UpdateCmdButton(localClientNum, button_bits, 14, 2, 1);
        }
    }
    else if ( (ps_otherFlags & 2) != 0 )
    {
        if ( nextSpecMode )
        {
            thirdPerson = cgameGlob->renderingThirdPerson != TP_FOR_MODEL;
            CG_SetThirdPerson(thirdPerson);
            Dvar_SetBool((dvar_s *)cg_spectateThirdPerson, thirdPerson);
        }
        if ( cgameGlob->inKillCam )
            CG_SetThirdPerson(0);
        else
            CG_SetThirdPerson(cg_spectateThirdPerson->current.color[0]);
        CL_UpdateCmdButton(localClientNum, button_bits, 14, -1, 1);
    }
    else
    {
        if ( nextSpecMode )
        {
            CG_SetThirdPerson(1);
            Dvar_SetBool((dvar_s *)cg_spectateThirdPerson, 1);
        }
        CL_UpdateCmdButton(localClientNum, button_bits, 14, 0, 1);
    }
    if ( (ps_otherFlags & 0xA) != 0 )
    {
        if ( CL_KeyActiveOrPressed(localClientNum, 0x10u) || CL_KeyActiveOrPressed(localClientNum, 0xFu) )
            CG_ResetSpectatorViewAngles(localClientNum);
        CL_UpdateCmdButton(localClientNum, button_bits, 16, 11, 1);
        CL_UpdateCmdButton(localClientNum, button_bits, 15, 0, 1);
        if ( cgameGlob->renderingThirdPerson == TP_FOR_MODEL )
            CL_ClampViewAngle(localClientNum, 0, -90.0, 90.0);
    }
    else
    {
        CL_UpdateCmdButton(localClientNum, button_bits, 16, 0, 1);
        CL_UpdateCmdButton(localClientNum, button_bits, 15, 0, 1);
    }
}

bool __cdecl CG_ShouldUpdateViewAngles(int localClientNum)
{
    if ( Demo_IsPlaying() )
        return Demo_ShouldUpdateViewAngles(localClientNum);
    return !Key_IsCatcherActive(localClientNum, -1) || UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD;
}

char __cdecl CG_TranslateGamepadButton(int localClientNum, int button, const char *cmd)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    if ( !button
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_gamepad.cpp", 211, 0, "%s", "button") )
    {
        __debugbreak();
    }
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_gamepad.cpp", 212, 0, "%s", "cmd") )
        __debugbreak();
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->inKillCam )
        return CG_GamepadKillCamInput(localClientNum, button, cmd);
    if ( cgameGlob->predictedPlayerState.pm_type == 4 || (cgameGlob->predictedPlayerState.otherFlags & 2) != 0 )
        return CG_GamepadSpectatorInput(localClientNum, button, cmd);
    return 0;
}

char __cdecl CG_GamepadSpectatorInput(int localClientNum, int button, const char *cmd)
{
    const char *v3; // eax
    const char *v5; // eax
    const char *v6; // eax
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.pm_type != 4
        && (cgameGlob->predictedPlayerState.otherFlags & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame_mp\\cg_gamepad.cpp",
                    167,
                    0,
                    "%s",
                    "(cgameGlob->predictedPlayerState.pm_type == PM_SPECTATOR) || (cgameGlob->predictedPlayerState.otherFlags & POF_FOLLOW)") )
    {
        __debugbreak();
    }
    switch ( button )
    {
        case 5:
            v3 = va("+specPrev %i\n-specPrev %i\n", 5, 5);
            Cbuf_AddText(localClientNum, v3);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "CyclePrev", "Flash", 1);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "SpectatorName", "Flash", 1);
            UI_AnimateItem(
                localClientNum,
                &cgDC[localClientNum],
                "spectatingPlayerCard",
                "spectatingPlayerSlide",
                "StartRight",
                1);
            return 1;
        case 6:
            v5 = va("+specNext %i\n-specNext %i\n", 6, 6);
            Cbuf_AddText(localClientNum, v5);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "CycleNext", "Flash", 1);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "SpectatorName", "Flash", 1);
            UI_AnimateItem(
                localClientNum,
                &cgDC[localClientNum],
                "spectatingPlayerCard",
                "spectatingPlayerSlide",
                "StartLeft",
                1);
            return 1;
        case 4:
            v6 = va("+toggleSpec %i\n-toggleSpec %i\n", 4, 4);
            Cbuf_AddText(localClientNum, v6);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "SpecViewToggle", "Flash", 1);
            return 1;
        default:
            return CG_GamepadCheckStartAndBack(localClientNum, button, cmd);
    }
}

char __cdecl CG_GamepadCheckStartAndBack(int localClientNum, int button, const char *cmd)
{
    const char *v3; // eax

    if ( (button != 14 || !*cmd) && (button != 15 || !*cmd) )
        return 0;
    v3 = va("%s %i\n", cmd, button);
    Cbuf_AddText(localClientNum, v3);
    return 1;
}

char __cdecl CG_GamepadKillCamInput(int localClientNum, int button, const char *cmd)
{
    const char *v3; // eax

    if ( button != 3 )
        return CG_GamepadCheckStartAndBack(localClientNum, button, cmd);
    v3 = va("+usereload %i\n-usereload %i\n", 3, 3);
    Cbuf_AddText(localClientNum, v3);
    return 1;
}


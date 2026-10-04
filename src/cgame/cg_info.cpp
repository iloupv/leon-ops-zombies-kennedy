#include "cg_info.h"
#include <cgame_mp/cg_local_mp.h>
#include <client_mp/cl_scrn_mp.h>
#include <cgame_mp/cg_servercmds_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <win32/win_shared.h>
#include <database/db_file_load.h>

void __cdecl CG_LoadingString(int localClientNum, const char *s)
{
    bool v2; // bl
    const char *v3; // eax

    v2 = *s != 0;
    CG_GetLocalClientGlobals(localClientNum)->isLoading = v2;
    if ( s && *s )
    {
        v3 = va("LOADING... %s\n", s);
        Com_Printf(14, v3);
    }
    SCR_UpdateLoadScreen();
}

void __cdecl CG_DrawInformation(int localClientNum)
{
    int v1; // [esp+1Ch] [ebp-30h]
    unsigned __int8 (*v2)(void); // [esp+20h] [ebp-2Ch]
    bool serverLoading; // [esp+27h] [ebp-25h]
    Font_s *font; // [esp+2Ch] [ebp-20h]
    float width; // [esp+30h] [ebp-1Ch]
    char *s; // [esp+38h] [ebp-14h]
    char *sa; // [esp+38h] [ebp-14h]
    const char *dots; // [esp+3Ch] [ebp-10h]

    serverLoading = CL_IsServerLoadingMap();
    if ( serverLoading )
    {
        if ( !CL_IsWaitingOnServerToLoadMap(localClientNum) )
        {
            CG_CloseScriptMenu(localClientNum, 0);
            UI_CloseAllMenus(localClientNum);
            CL_SetWaitingOnServerToLoadMap(localClientNum, 1);
        }
    }
    else
    {
        CL_SetWaitingOnServerToLoadMap(localClientNum, 0);
    }
    CL_GetConfigString(0);
    UI_DrawMapLevelshot(localClientNum);
    if ( useFastFile->current.enabled )
        v2 = (unsigned __int8 (*)(void))CG_IsShowingProgress_FastFile;
    else
        v2 = (unsigned __int8 (*)(void))CG_IsShowingProgress_LoadObj;
    if ( !v2() && serverLoading && g_waitingForServer )
    {
        font = UI_GetFontHandle(&scrPlaceFull, 0, 0.5);
        v1 = ((int)Sys_Milliseconds() / 500) & 3;
        switch ( v1 )
        {
            case 1:
                dots = ".";
                break;
            case 2:
                dots = "..";
                break;
            case 3:
                dots = "...";
                break;
            default:
                dots = "";
                break;
        }
        s = UI_SafeTranslateString("CGAME_WAITINGFORSERVERLOAD");
        width = (float)UI_TextWidth(s, 0, font, 0.5);
        sa = va("%s%s", s, dots);
        UI_DrawText(
            &scrPlaceView[localClientNum],
            sa,
            0x7FFFFFFF,
            font,
            (float)(640.0 - width) * 0.5,
            439.0,
            0,
            0,
            0.5,
            colorWhite,
            3);
    }
}

bool __cdecl CG_IsShowingProgress_LoadObj()
{
    return com_expectedHunkUsage > 0;
}

bool __cdecl CG_IsShowingProgress_FastFile()
{
    return DB_GetLoadedFraction() > 0.0;
}


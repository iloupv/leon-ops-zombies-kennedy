#include "live_win_common.h"
#include <qcommon/com_clients.h>
#include "live_win.h"
#include <DW/dwNet.h>
#include "live_groups_dw.h"
#include <DW/MatchRecorder.h>
#include "live_leaderboard.h"
#include "live_sessions_win.h"
#include "live_news.h"
#include "live_counter.h"
#include "live_pcache.h"
#include <client_mp/cl_main_pc_mp.h>
#include <win32/win_shared.h>
#include "live.h"
#include <qcommon/common.h>
#include <universal/assertive.h>
#include <qcommon/threads.h>

#include <DW/dwLogOn_pc.h>
#include <DW/MatchMakingInfo_win32.h>

const char *s_comErrorString;
bool s_shouldComError;

bool dw_disconnect_detected;

int sessiontimer;

extern const dvar_t *live_service; // BO1TODO: remove these later
extern const dvar_t *dw_dupe_key;
char __cdecl Live_Frame_MP(int localControllerIndex)
{
#ifdef BO1_LIVE
    bool v2; // [esp+7h] [ebp-21h]
    char *disconnectMessage; // [esp+Ch] [ebp-1Ch] BYREF
    int LogOnStatus; // [esp+10h] [ebp-18h]
    DWOnlineStatus newOnlineStatus; // [esp+14h] [ebp-14h]
    bool justSignedIn; // [esp+1Bh] [ebp-Dh]
    DWOnlineStatus onlineStatus; // [esp+1Ch] [ebp-Ch]
    bool online; // [esp+23h] [ebp-5h]
    int elapsed; // [esp+24h] [ebp-4h]

    elapsed = 0;
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_win_common.cpp",
                    1004,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !Sys_IsMainThread() )
        return 0;
    if ( s_shouldComError )
    {
        s_shouldComError = 0;
        Com_Error(ERR_DROP, s_comErrorString);
    }

    if ( !live_service->current.enabled )
        return 0;

    if ( dw_dupe_key->current.enabled )
        return 0;

    if ( dw_disconnect_detected )
    {
        Live_DemonwareDisconnectCleanup(0);
        return 0;
    }
    else
    {
        dw_disconnect_detected = 0;
        onlineStatus = (DWOnlineStatus)dwGetLogOnStatus(localControllerIndex);
        switch ( onlineStatus )
        {
            case DW_LIVE_ERROR:
                Live_DemonwareDisconnectCleanup(0);
                break;
            case DW_LIVE_DISCONNECTED:
                Com_DPrintf(16, "Connecting to Demonware...\n");
                Live_UpdateUiPopup("popup_connecting_dw", 0);
                dwLogOnStart(localControllerIndex);
                break;
            case DW_LIVE_CONNECTING:
                dwLogOnComplete();
                newOnlineStatus = (DWOnlineStatus)dwGetLogOnStatus(localControllerIndex);
                if ( newOnlineStatus == DW_LIVE_DISCONNECTED )
                {
                    Com_Printf(16, "Failed to log on.\n");
                    Live_UpdateUiPopup("popup_cannot_connect_to_dw", 0);
                    dw_disconnect_detected = 1;
                }
                break;
            case DW_LIVE_CONNECTED:
                elapsed = Sys_Milliseconds() - sessiontimer;
                sessiontimer = Sys_Milliseconds();
                dwLobbyPump(localControllerIndex);
                CL_CACValidate_Frame();
                PCache_Update(localControllerIndex);
                LiveCounter_Update(localControllerIndex);
                LiveNews_UpdateCountersTicker();
                LiveStorage_CheckOngoingSessionTasks();
                Live_CheckOngoingSessionTasks();
                LB_CheckOngoingTasks();
                LiveGroups_Update(localControllerIndex);
                Live_CheckOngoingMatchRecordingTasks();
                //if ( MatchMakingInfo::doUpdate(g_matchmakingInfo, elapsed, 0xBB8u, 0x2BF20u) )
                if ( g_matchmakingInfo->doUpdate(elapsed, 0xBB8u, 0x2BF20u) )
                    Session_Modify(0, &g_serverSession, 0, g_serverSession.publicSlots, g_serverSession.privateSlots);
                LogOnStatus = dwGetLogOnStatus(localControllerIndex);
                if ( LogOnStatus == 2 )
                    Com_Printf(16, "Lost connection to lobby server.\n");
                break;
            default:
                break;
        }
        online = dwGetLogOnStatus(localControllerIndex) == 4;
        dwNetStart(online);
        if ( online != Dvar_GetBool("dw_loggedin") )
            Dvar_SetBoolByName("dw_loggedin", online);
        v2 = dwGetLogOnStatus(localControllerIndex) == 4 && onlineStatus != DW_LIVE_CONNECTED;
        justSignedIn = v2;
        if ( v2 )
            Live_UserSignedInToLive(localControllerIndex, &disconnectMessage);
        if ( dwGetLogOnStatus(localControllerIndex) == 4 )
        {
            Live_FindSessionsPump();
            return 1;
        }
        else
        {
            return 0;
        }
    }
#else
return 0;
#endif
}

extern const dvar_t *dw_popup; // BO1TODO: remove later
void __cdecl Live_UpdateUiPopup(const char *popupname, bool reset)
{
    int v2; // eax

    if ( cls.uiStarted )
    {
        Com_Printf(16, "Live_UpdateUiPopup: %s\n", popupname);
        if ( reset )
            v2 = I_stricmp(dw_popup->current.string, "");
        else
            v2 = I_stricmp(dw_popup->current.string, popupname);
        if ( v2 )
        {
            if ( reset )
                Dvar_SetString((dvar_s *)dw_popup, "");
            else
                Dvar_SetString((dvar_s *)dw_popup, popupname);
        }
    }
    else
    {
        Com_Printf(16, "%s\n", popupname);
    }
}

void __cdecl Live_ShowPlayerProfile(int localClientNum, unsigned __int64 playerXUID, char *playerName)
{
    int ControllerIndex; // eax
    __int64 v4; // rax
    char *v5; // eax
    uiInfo_s *uiInfo; // [esp+8h] [ebp-8h]
    menuDef_t *menu; // [esp+Ch] [ebp-4h]

    ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    LODWORD(v4) = Live_GetXuid(ControllerIndex);
    if ( playerXUID && playerXUID != v4 )
    {
        v5 = va("%11d", (unsigned int)playerXUID);
        Dvar_SetStringByName("selectedPlayerXuid", v5);
        Dvar_SetStringByName("selectedFriendName", playerName);
        uiInfo = UI_GetInfo(localClientNum);
        menu = Menus_FindByName(&uiInfo->uiDC, "menu_playercard");
        if ( menu )
        {
            menu->openSlideDirection = 2;
            Menus_Open(localClientNum, &uiInfo->uiDC, menu);
        }
    }
}

void __cdecl SocketRouter_EmergencyFrame()
{
    Sys_IsMainThread();
}


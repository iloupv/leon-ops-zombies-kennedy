#include "live.h"
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame/cg_compass.h>
#include "live_win.h"
#include "live_storage_pub.h"
#include "live_storage.h"
#include <qcommon/common.h>
#include <demo/demo_playback.h>
#include <qcommon/com_gamemodes.h>
#include <DW/dwLogOn_pc.h>
#include <DemonWare/bdLobbyService.h>
#include <DW/dwUtils.h>

int __cdecl Live_CountGuestsInUse()
{
    return 0;
}

void __cdecl Live_ShowConnectingDialog()
{
    int controllerIndex; // [esp+0h] [ebp-8h]
    bool isUserSignedInToLive; // [esp+4h] [ebp-4h]
    bool isUserSignedInToDemonware; // [esp+5h] [ebp-3h]
    bool allInfoDownloaded; // [esp+6h] [ebp-2h]
    bool isUserGuest; // [esp+7h] [ebp-1h]

    allInfoDownloaded = 1;
    for ( controllerIndex = 0; controllerIndex < 1; ++controllerIndex )
    {
        if ( CL_LocalClient_IsCUIFlagSet(controllerIndex, 32) )
            return;
        isUserGuest = CG_IsShowingZombieMap();
        isUserSignedInToLive = Live_IsUserSignedInToLive();
        isUserSignedInToDemonware = Live_IsUserSignedInToDemonware(controllerIndex);
        if ( isUserSignedInToLive
            && !isUserGuest
            && (!isUserSignedInToDemonware
             || !LiveStorage_DoWeHavePlaylists()
             || !LiveStorage_DoWeHaveAllStats(controllerIndex)
             || !LiveStorage_IsTimeSynced()
             || !LiveStorage_DoWeHaveContracts()) )
        {
            allInfoDownloaded = 0;
            break;
        }
    }
    if ( !onlinegame->current.enabled || allInfoDownloaded )
    {
        if ( Menu_IsMenuOpenAndVisible(0, "popup_inliveconnectingtodw") )
            UI_CloseMenu(0, "popup_inliveconnectingtodw");
    }
    else if ( !Menu_IsMenuOpenAndVisible(0, "popup_inliveconnectingtodw") )
    {
        UI_OpenMenu(0, "popup_inliveconnectingtodw");
    }
    if ( allInfoDownloaded && Menu_IsMenuOpenAndVisible(0, "popup_connectingtodw") )
        UI_CloseMenu(0, "popup_connectingtodw");
}

void __cdecl Live_DemonwareDisconnectCleanup(int localControllerIndex)
{
#ifdef BO1_LIVE
    bdLobbyService *lobbyService; // [esp+0h] [ebp-4h]

    Com_GameMode_ResetGameModes();
    lobbyService = dwGetLobby(localControllerIndex);
    DWDisconnectLobby(lobbyService);
    if ( onlinegame->current.enabled )
    {
        Dvar_SetBool((dvar_s *)onlinegame, 0);
        Com_Error(ERR_DROP, "EXE_DEMONWARE_DISCONNECT");
    }
#endif
}

bool __cdecl Live_IsPublicOnlineMatch()
{
    if ( Demo_IsPlaying() )
        return Demo_IsPublicOnlineMatch();
    else
        return Com_GameMode_IsPublicOnlineGame();
}

bool __cdecl Live_IsWagerMatch()
{
    if ( Demo_IsPlaying() )
        return Demo_IsWagerMatch();
    else
        return Com_GameMode_IsGameMode(GAMEMODE_WAGER_MATCH);
}

bool __cdecl Live_IsCombatTrainingMatch()
{
    if ( Demo_IsPlaying() )
        return Demo_IsCombatTrainingMatch();
    else
        return Com_GameMode_IsGameMode(GAMEMODE_BASIC_TRAINING);
}

void __cdecl Live_Init()
{
    Live_InitPlatform();
}


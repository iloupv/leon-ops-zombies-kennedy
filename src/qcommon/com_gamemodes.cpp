#include "com_gamemodes.h"
#include <universal/dvar.h>
#include <game_mp/g_main_mp.h>
#include <universal/assertive.h>

const dvar_s *onlinegame;
//const dvar_s *xblive_privatematch;
//const dvar_s *xblive_basictraining;
//const dvar_s *xblive_theater;
//const dvar_s *xblive_wagermatch;
//const dvar_s *xblive_clanmatch;

bool __cdecl Com_GameMode_IsOnlineGame()
{
    return onlinegame && onlinegame->current.enabled;
}

bool __cdecl Com_GameMode_IsPrivateOnlineGame()
{
    if ( !onlinegame || !onlinegame->current.enabled )
        return 0;
    if ( xblive_privatematch && xblive_privatematch->current.enabled )
        return 1;
    if ( xblive_basictraining && xblive_basictraining->current.enabled )
        return 1;
    return xblive_theater && xblive_theater->current.enabled;
}

bool __cdecl Com_GameMode_IsPublicOnlineGame()
{
    return Com_GameMode_IsOnlineGame() && !Com_GameMode_IsPrivateOnlineGame();
}

bool __cdecl Com_GameMode_IsGameMode(eGameModes mode)
{
    bool result; // al

    ValidateGameModes();
    switch ( mode )
    {
        case GAMEMODE_SYSTEMLINK:
            result = 0;
            break;
        case GAMEMODE_LOCAL_SPLITSCREEN:
            result = 0;
            break;
        case GAMEMODE_FIRST_ONLINE_GAMEMODE:
            result = Com_GameMode_IsOnlineGame()
                        && !Com_GameMode_IsPrivateOnlineGame()
                        && (!xblive_wagermatch || !xblive_wagermatch->current.enabled);
            break;
        case GAMEMODE_FIRST_PRIVATE_ONLINE_GAMEMODE:
            result = xblive_privatematch && xblive_privatematch->current.enabled;
            break;
        case GAMEMODE_WAGER_MATCH:
            result = xblive_wagermatch && xblive_wagermatch->current.enabled;
            break;
        case GAMEMODE_BASIC_TRAINING:
            result = xblive_basictraining && xblive_basictraining->current.enabled;
            break;
        case GAMEMODE_THEATER:
            result = xblive_theater && xblive_theater->current.enabled;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}

char ValidateGameModes()
{
    char result; // al
    bool anySet; // [esp+1h] [ebp-1h]
    bool anySeta; // [esp+1h] [ebp-1h]
    bool anySetb; // [esp+1h] [ebp-1h]

    result = G_ExitAfterToolComplete();
    if ( !result )
    {
        anySet = xblive_privatematch->current.enabled;
        if ( anySet
            && xblive_basictraining->current.enabled
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_gamemodes.cpp",
                        38,
                        0,
                        "%s",
                        "!anySet || !xblive_basictraining->current.enabled") )
        {
            __debugbreak();
        }
        anySeta = xblive_basictraining->current.enabled || anySet;
        if ( anySeta
            && xblive_theater->current.enabled
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_gamemodes.cpp",
                        42,
                        0,
                        "%s",
                        "!anySet || !xblive_theater->current.enabled") )
        {
            __debugbreak();
        }
        anySetb = xblive_theater->current.enabled || anySeta;
        if ( anySetb
            && xblive_wagermatch->current.enabled
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_gamemodes.cpp",
                        45,
                        0,
                        "%s",
                        "!anySet || !xblive_wagermatch->current.enabled") )
        {
            __debugbreak();
        }
        result = xblive_wagermatch->current.enabled || anySetb;
        if ( xblive_privatematch && (result = (char)xblive_privatematch, xblive_privatematch->current.enabled) )
        {
            if ( !onlinegame || (result = onlinegame->current.enabled) == 0 )
            {
                result = Assert_MyHandler(
                                     "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_gamemodes.cpp",
                                     52,
                                     0,
                                     "%s",
                                     "onlinegame && onlinegame->current.enabled");
                if ( !result )
                    __debugbreak();
            }
        }
        else if ( xblive_basictraining && (result = (char)xblive_basictraining, xblive_basictraining->current.enabled) )
        {
            if ( !onlinegame || (result = onlinegame->current.enabled) == 0 )
            {
                result = Assert_MyHandler(
                                     "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_gamemodes.cpp",
                                     57,
                                     0,
                                     "%s",
                                     "onlinegame && onlinegame->current.enabled");
                if ( !result )
                    __debugbreak();
            }
        }
        else if ( xblive_theater && (result = (char)xblive_theater, xblive_theater->current.enabled) )
        {
            if ( !onlinegame || (result = onlinegame->current.enabled) == 0 )
            {
                result = Assert_MyHandler(
                                     "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_gamemodes.cpp",
                                     61,
                                     0,
                                     "%s",
                                     "onlinegame && onlinegame->current.enabled");
                if ( !result )
                    __debugbreak();
            }
        }
        else if ( xblive_wagermatch )
        {
            result = (char)xblive_wagermatch;
            if ( xblive_wagermatch->current.enabled && (!onlinegame || (result = onlinegame->current.enabled) == 0) )
            {
                result = Assert_MyHandler(
                                     "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_gamemodes.cpp",
                                     65,
                                     0,
                                     "%s",
                                     "onlinegame && onlinegame->current.enabled");
                if ( !result )
                    __debugbreak();
            }
        }
    }
    return result;
}

void __cdecl Com_GameMode_ResetGameModes()
{
    Dvar_SetBool((dvar_s *)xblive_privatematch, 0);
    Dvar_SetBool((dvar_s *)xblive_clanmatch, 0);
    Dvar_SetBool((dvar_s *)xblive_wagermatch, 0);
    Dvar_SetBool((dvar_s *)xblive_basictraining, 0);
    Dvar_SetBool((dvar_s *)xblive_theater, 0);
}


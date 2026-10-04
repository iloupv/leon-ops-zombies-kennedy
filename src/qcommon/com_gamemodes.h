#pragma once

#include <universal/dvar.h>

enum eGameModes : __int32
{                                       // XREF: ?Com_GameMode_IsGameMode@@YA_NW4eGameModes@@@Z/r
    GAMEMODE_SYSTEMLINK            = 0x0,
    GAMEMODE_LOCAL_SPLITSCREEN     = 0x1,
    GAMEMODE_FIRST_ONLINE_GAMEMODE = 0x2,
    GAMEMODE_PUBLIC_MATCH          = 0x2,
    GAMEMODE_FIRST_PRIVATE_ONLINE_GAMEMODE = 0x3,
    GAMEMODE_PRIVATE_MATCH         = 0x3,
    GAMEMODE_WAGER_MATCH           = 0x4,
    GAMEMODE_BASIC_TRAINING        = 0x5,
    GAMEMODE_THEATER               = 0x6,
    GAMEMODE_LAST_ONLINE_GAMEMODE  = 0x7,
};


bool __cdecl Com_GameMode_IsOnlineGame();
bool __cdecl Com_GameMode_IsPrivateOnlineGame();
bool __cdecl Com_GameMode_IsPublicOnlineGame();
bool __cdecl Com_GameMode_IsGameMode(eGameModes mode);
char ValidateGameModes();
void __cdecl Com_GameMode_ResetGameModes();

extern const dvar_s *onlinegame;
extern const dvar_s *xblive_privatematch;
extern const dvar_s *xblive_basictraining;
extern const dvar_s *xblive_theater;
extern const dvar_s *xblive_wagermatch;
extern const dvar_s *xblive_clanmatch;
#pragma once
#include <client_mp/cl_ui_pc_mp.h>

enum LiveSteam_PersonaState : __int32
{                                       // XREF: ?LiveSteam_GetFriendPersonaState@@YA?AW4LiveSteam_PersonaState@@_K@Z/r
    LIVE_STEAM_PERSONA_STATE_OFFLINE = 0x0,
    LIVE_STEAM_PERSONA_STATE_ONLINE  = 0x1,
    LIVE_STEAM_PERSONA_STATE_BUSY    = 0x2,
    LIVE_STEAM_PERSONA_STATE_SNOOZE  = 0x3,
};

int __cdecl LiveSteam_GetNumFriends();
int __cdecl LiveSteam_GetNumFriendsOnTheServer();
unsigned __int64 __cdecl LiveSteam_GetFriendXuid(int index);
void __cdecl LiveSteam_GetFriendPersonaName(int index, char *buf, int size);
void __cdecl LiveSteam_GetFriendOnTheServerPersonaName(int index, char *buf, int size);
int __cdecl LiveSteam_GetFriendPersonaState(unsigned __int64 xuid);
int __cdecl LiveSteam_GetFriendPersonaStateByIndex(int index);
char __cdecl Friends_Update();
int __cdecl Friends_GetCount(int __formal, int type);
char __cdecl Friends_GetByIndex(int controllerIndex, int filter, int index, FriendInfo *info);
char __cdecl Friends_GetByID(int controllerIndex, unsigned __int64 id, FriendInfo *info);
bool __cdecl Friend_GetByName(int controllerIndex, const char *friendName, unsigned __int64 *uid);
void __cdecl Friends_GetPeopleOnServer();
void __cdecl Friend_PrintNamesOnServer_f();
void __cdecl Friends_Init();
void __cdecl Live_GetRecentPlayerName(unsigned __int64 uid, char *buf, int bufsize);

//CSteamID *__thiscall CSteamID::CSteamID(CSteamID *this);
//bool __thiscall CSteamID::IsValid(CSteamID *this);

//int __cdecl _GetSortedFriendIndex(int index);
//int __cdecl _GetNumFriends<4>();
//int __cdecl _GetNumFriends<16>();
//unsigned __int64 __cdecl _GetFriendXuid<4>(int index);
//void __cdecl _GetFriendPersonaName<4>(int index, char *buf, int size);
//void __cdecl _GetFriendPersonaName<16>(int index, char *buf, int size);
//char __cdecl _GetFriendPersonaNameByXuid<4>(unsigned __int64 xuid, char *buf, int size);
//
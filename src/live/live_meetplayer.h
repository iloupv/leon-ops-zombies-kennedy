#pragma once

#include <universal/q_shared.h>
#include <Windows.h>

struct XuidInfo // sizeof=0x30
{                                       // XREF: .data:recentPlayerInfo/r
    unsigned __int64 playerXuids;       // XREF: UI_FeederItemText_CommmonPlayerListHandler+CE/r
                                        // UI_FeederItemText_CommmonPlayerListHandler+D6/r ...
    char gamertag[32];                  // XREF: UI_FeederItemText_CommmonPlayerListHandler+DF/o
                                        // UI_FeederDataExtended_RecentPlayers+E9/o ...
    _FILETIME systemTime;
};

struct __declspec(align(8)) MetPlayerInfo // sizeof=0x970
{                                       // XREF: .data:metPlayersXuidinfo/r
    unsigned __int8 version;            // XREF: LiveMeetPlayer_ResetData(int)+2A/w
                                        // LiveMeetPlayer_CheckDownloadedVersionAndResetData(int)+13/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    XuidInfo playersXuidInfo[50];       // XREF: LiveMeetPlayer_CheckDownloadedVersionAndResetData(int)+88/r
                                        // LiveMeetPlayer_GetXUIDPosInMetPlayerList(int,unsigned __int64)+4B/r ...
    unsigned __int8 metPlayerCount;     // XREF: LiveMeetPlayer_CheckDownloadedVersionAndResetData(int)+61/r
                                        // LiveMeetPlayer_GetXUIDPosInMetPlayerList(int,unsigned __int64)+21/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl LiveMeetPlayer_DownloadMetPlayersList(int localControllerIndex);
void __cdecl LiveMeetPlayer_ClearUploadFlags(int controllerIndex);
bool __cdecl LiveMeetPlayer_UploadMetPlayerListNeeded(int controllerIndex);
void __cdecl LiveMeetPlayer_ResetData(int controllerIndex);
void __cdecl LiveMeetPlayer_CheckDownloadedVersionAndResetData(int localControllerIndex);
void __cdecl LiveMeetPlayer_UploadMetPlayerList(int localControllerIndex);
void __cdecl LiveMeetPlayer_Init();
int __cdecl LiveMeetPlayer_GetXUIDPosInMetPlayerList(int localControllerIndex, unsigned __int64 currentPlayerXuid);
int __cdecl LiveMeetPlayer_GetNoOfMetPlayers(int localControllerIndex);
void __cdecl LiveMeetPlayer_SortMetPlayers(int localControllerIndex, unsigned int noOfPlayers);
int __cdecl LiveMeetPlayer_MetPlayerCompare(_QWORD *arg0, _QWORD *arg1);
void __cdecl LiveMeetPlayer_UpdateMeetPlayerList(
                int localControllerIndex,
                unsigned __int64 playeruid,
                const char *playername);
void __cdecl LiveMeetPlayer_SetCurrentTime(_FILETIME *currentTime);
char __cdecl LiveMeetPlayer_AddPlayerAtSlot(
                int localControllerIndex,
                unsigned __int64 playeruid,
                const char *playername,
                int slot);
unsigned int __cdecl LiveMeetPlayer_GetLastMetTime(int localControllerIndex, int index);
XuidInfo *__cdecl LiveMeetPlayer_GetMetFriendInfo(XuidInfo *result, int localControllerIndex, int index);
char __cdecl LiveMeetPlayer_GetRecentPlayerInfoByID(
                int controllerIndex,
                unsigned __int64 id,
                XuidInfo *recentMetPlayerInfo);
unsigned int __cdecl LiveMeetPlayer_GetRecentPlayerIndexByID(int controllerIndex, unsigned __int64 id);
void __cdecl LiveMeetPlayer_UpdatePlayerSession(unsigned __int64 playerUid, unsigned __int64 sessionUid);
int __cdecl LiveMeetPlayer_GetPlayerSessionByID(unsigned __int64 playerUid);

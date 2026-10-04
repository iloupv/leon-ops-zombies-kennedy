#pragma once
#include <universal/q_parse.h>
#include <universal/dvar.h>
#include "ui_shared.h"

enum sscType_t : __int32
{                                       // XREF: serverStatusDvar_t/r
    SSC_STRING = 0x0,
    SSC_YESNO = 0x1,
    SSC_GAMETYPE = 0x2,
    SSC_MAPNAME = 0x3,
};

struct serverStatusDvar_t // sizeof=0xC
{
    const char *name;
    const char *altName;
    sscType_t type;
};


parseInfo_t *__cdecl UI_GetMapRotationToken();
void __cdecl UI_RegisterDvars_PC();
const char *__cdecl UI_GetNetSources(unsigned int source);
void __cdecl UI_SelectCurrentGameType();
int __cdecl UI_GetListIndexFromMapIndex(int testMapIndex);
int __cdecl UI_MapCountByGameType();
void __cdecl UI_SelectCurrentMap(int localClientNum);
void __cdecl UI_UseAltColorPalette(char *buffer);
bool __cdecl ValidGamePadButtonIcon(unsigned int letter);
void __cdecl UI_RemoveServerFromDisplayList(int num);
char __cdecl UI_IsServerStatusDvar(const char *dvarname);
int __cdecl UI_GetServerStatusInfo(char *serverAddress, serverStatusInfo_s *info);
void __cdecl UI_SortServerStatusInfo(serverStatusInfo_s *info);
int __cdecl UI_GetServerStatusInfoScoreBoard(char *serverAddress, serverStatusInfo_s *info);
void __cdecl UI_SortServerStatusScoreBoardInfo(serverStatusInfo_s *info);
int __cdecl ScoreBar_CompareScores(const char **sv1, const char **sv2);
void __cdecl UI_RefreshServers();
void __cdecl UI_BuildFindPlayerList();
char *__cdecl stristr(char *str, char *charset);
char *__cdecl UI_SelectedMap(int index, int *actual);
int __cdecl UI_GameType_HandleKey(int flags, int key, int resetMap);
int __cdecl UI_NetGameType_HandleKey(int flags, int key);
bool __cdecl UI_IsMapActive(int mapIndex);
int UI_SelectFirstActiveMap();
void __cdecl UI_SelectListIndexForMapIndex(int mapIndex);
int __cdecl UI_JoinGameType_HandleKey(int flags, int key);
int __cdecl UI_JoinMod_HandleKey(int flags, int key);
void __cdecl UI_NetSource_UpdateDisplayList(int source);
int __cdecl UI_NetSource_HandleKey(int flags, int key);
int __cdecl UI_NetFilter_HandleKey(int flags, int key);
void __cdecl UI_DrawGameType(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle);
void __cdecl UI_DrawNetGameType(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle);
void __cdecl UI_DrawJoinGameType(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle);
void __cdecl UI_DrawJoinMod(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle);
void __cdecl UI_DrawNetFilter(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle);
void __cdecl UI_DrawNetSource(
                int contextIndex,
                const rectDef_s *rect,
                Font_s *font,
                float scale,
                const float *color,
                int textStyle);
Material *__cdecl UI_GetLevelShot(int index);
void __cdecl UI_DrawMapPreview(int contextIndex, const rectDef_s *rect, const float *color);
bool __cdecl UI_LodMods_ShouldSkipMap(const char *dir);
void __cdecl UI_LoadMods();
void __cdecl UI_AcceptFriend();
void __cdecl UI_DeclineFriend();
void __cdecl UI_AcceptInvite();
void __cdecl UI_ClearMods();

extern const dvar_t *ui_joinGameType;
extern const dvar_t *ui_netGameTypeName;
extern const dvar_t *ui_dedicated;
extern const dvar_t *ui_currentNetMap;
extern const dvar_t *ui_browserShowFull;
extern const dvar_t *ui_browserShowEmpty;
extern const dvar_t *ui_browserShowPassword;
extern const dvar_t *ui_browserShowPure;
extern const dvar_t *ui_browserMod;
extern const dvar_t *ui_browserShowDedicated;
extern const dvar_t *ui_browserFriendlyfire;
extern const dvar_t *ui_browserKillcam;
extern const dvar_t *ui_browserVoiceChat;
extern const dvar_t *ui_browserGameMode;
extern const dvar_t *ui_friendNameNew;
extern const dvar_t *ui_friendSelectedInd;
extern const dvar_t *ui_friendPendingSelectedInd;
extern const dvar_t *ui_inviteSelectedInd;
extern const dvar_t *ui_inviteScreen;
extern const dvar_t *ui_browserPlayerCount;
extern const dvar_t *ui_browserDedicatedServerCount;
extern const dvar_t *ui_browserShowToolTip;
extern const dvar_t *ui_browserShowMapTip;
extern const dvar_t *ui_browserToolTip;
extern const dvar_t *ui_browserShowInfo;
extern const dvar_t *ui_browserShowPunkBuster;
extern const dvar_t *ui_allowConsole;
extern const dvar_t *ui_allowFov;
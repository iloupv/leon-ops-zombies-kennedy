#pragma once

#ifdef BO1_DW

#include <bgame/bg_public.h>
#include <ddl/ddl_api.h>

enum taskCompleteResults : __int32;


void __cdecl MatchRecorder_ClearDWOverlappedTasks();
void __cdecl MatchRecord_InitMatchData();
int __cdecl MatchRecord_InitMatchDataInternal(char *inputBuffer, int buffSize);
void __cdecl MatchRecorderDDLSetInt(ddlState_t *searchState, const char *name, unsigned int value);
bool __cdecl MatchRecord_IsHeatMapDataGenerated();
unsigned int __cdecl MatchRecord_GetMapIndex();
unsigned int __cdecl MatchRecord_GetXUIDForPlayerSlot(unsigned int playerSlot);
unsigned int __cdecl MatchRecorderDDLGetInt64(ddlState_t *searchState, const char *name);
char __cdecl MatchRecord_GetPlayerInSlot(unsigned int playerSlot, ddlState_t *playerSlotState);
void __cdecl SaveTeamScores();
void __cdecl GenerateRecordedMatchData(int controllerIndex, char **matchRecordBuffer, int *matchRecordBufferSize);
bool __cdecl CompressBuffer(
                char *destBuffer,
                unsigned int *destBufferLen,
                char *sourceBuffer,
                unsigned int sourceBufferLen);
void __cdecl GenerateBinaryMatchSummary(unsigned int titleID);
unsigned int __cdecl MatchRecorderDDLGetInt(ddlState_t *searchState, const char *name);
unsigned int __cdecl MatchRecord_GetPlayerCount();
int __cdecl GetCurrentMapIndex();
int __cdecl GetCurrentPlaylistIndex();
void __cdecl FindBitchForPlayer(unsigned int attackingPlayerSlot);
unsigned int __cdecl MatchRecord_GetLifeCount();
char __cdecl MatchRecord_GetLifeInSlot(unsigned int lifeSlot, ddlState_t *lifeSlotState);
void __cdecl FindNemesisForPlayer(unsigned int attackingPlayerSlot);
void GenerateBinaryBreadcrumbData();
void ClearTeamScores();
void __cdecl MatchRecord_NormaliseHeatMap(
                int width,
                int height,
                unsigned __int8 *heatMap,
                unsigned __int8 *buffer,
                float maxVal);
unsigned __int8 __cdecl MatchRecord_ReadHeatMapPixel(
                int xPos,
                int yPos,
                int width,
                int height,
                unsigned __int8 *buffer);
void __cdecl MatchRecord_WriteHeatMapPixel(
                int xPos,
                int yPos,
                int width,
                int height,
                unsigned __int8 *buffer,
                unsigned __int8 pixelVal);
void __cdecl MatchRecord_GenerateHeatMapData(
                unsigned __int8 *buffer,
                int buffSize,
                int width,
                int height,
                unsigned __int64 xuid);
unsigned __int8 __cdecl MatchRecord_BlurHeatMap(
                int width,
                int height,
                unsigned __int8 *buffer,
                unsigned int bufferSize);
void __cdecl MatchRecord_SetPlayerDeathPosition(
                unsigned __int8 *buffer,
                int width,
                int height,
                float xScale,
                float yScale,
                int xPos,
                int yPos);
void __cdecl MatchRecord_GeneratePooledFileDetails(int controllerIndex);
int __cdecl MatchRecord_ParseDownloadedMatchData(int controllerIndex, char *inputBuffer, int buffsize);
void __cdecl MatchRecordSpawn(gclient_s *client);
void __cdecl MatchRecorderDDLSetInt64(ddlState_t *searchState, const char *name, unsigned __int64 value);
void __cdecl MatchRecordCopyExternalIP(ddlState_t playerState, unsigned int clientNum);
void __cdecl MatchRecordWriteItemStatsStart(ddlState_t playerState, unsigned int clientNum);
void __cdecl MatchRecord_SetLifeCount(__int16 currentLives);
unsigned int __cdecl MatchRecord_SearchCreateForPlayer(gclient_s *client, ddlState_t *playerState);
void __cdecl MatchRecorderDDLSetString(ddlState_t *searchState, const char *name, const char *value);
void __cdecl MatchRecord_SetPlayerCount(__int16 currentPlayers);
int __cdecl MatchRecord_SearchForPlayerSlotByXUID(unsigned __int64 xuid);
void __cdecl GetWorldLocation(
                float xPos,
                float yPos,
                float zPos,
                unsigned __int8 *xWorld,
                unsigned __int8 *yWorld,
                unsigned __int8 *zWorld);
void __cdecl MatchRecordHit(gclient_s *attackerClient, hitLocation_t hitLocation);
void __cdecl MatchRecordSetHitStat(
                ddlState_t playerState,
                gclient_s *attackerClient,
                hitLocation_t hitLocation,
                const char *hitLocStatName);
void __cdecl MatchRecordDeath(
                gclient_s *client,
                gclient_s *attackerClient,
                char *weaponName,
                hitLocation_t hitLocation);
void __cdecl MatchRecordPointsSpent(int controllerIndex, int amount, pointsSpent_t reasonType, int reasonIndex);
void __cdecl MatchRecordEnd(gclient_s *client);
void __cdecl MatchRecordWriteItemStatsEnd(ddlState_t playerState, unsigned int clientNum);
void __cdecl MatchRecordBegin();
void __cdecl MatchRecordPlayerDetails(gclient_s *client, const char *reason);
char __cdecl MatchRecordSetPlayerStat(gclient_s *client, const char *statName, unsigned int statValue);
void __cdecl MatchRecordMovement();
void __cdecl Live_CheckOngoingMatchRecordingTasks();
taskCompleteResults __cdecl MatchRecordEventComplete(int slot);

#endif
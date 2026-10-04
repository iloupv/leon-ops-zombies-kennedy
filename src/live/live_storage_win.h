#pragma once
#include "live_storage.h"
#include "live_leaderboard.h"


struct dwFileOperationInfo;

enum blobtype_t : __int32
{                                       // XREF: ?Live_CACValidate_DispatchMessage@@YA_N_KPAXI@Z/r
    BLOB_TYPE_CAC     = 0x0,
    BLOB_TYPE_GLOBAL  = 0x1,
    BLOB_TYPE_INVALID = 0x2,
};

enum sv_cacvalidate_state_t : __int32
{
    CAC_IDLE         = 0x0,
    CAC_WAITING      = 0x1,
    CAC_FETCHONE     = 0x2,
    CAC_FETCHTWO     = 0x3,
    CAC_WAITINGAGAIN = 0x4,
    CAC_WRITEONE     = 0x5,
    CAC_WRITETWO     = 0x6,
};

enum day_t : __int32
{                                       // XREF: ?SV_DoTimedStatsForClient@@YAXHPBDPAV?$LeaderBoardRow@$09@@H1H@Z/r
                                        // ?SV_DoTimedStatsForClient@@YAXHPBDPAV?$LeaderBoardRow@$09@@H1H@Z/r
    SUNDAY = 0x0,
    MONDAY = 0x1,
    TUESDAY = 0x2,
    WEDNESDAY = 0x3,
    THURSDAY = 0x4,
    FRIDAY = 0x5,
    SATURDAY = 0x6,
};

int __cdecl SystemTimeToInt();
void __cdecl LiveStorage_ResetStats(unsigned __int8 *buffer);
void __cdecl LiveStorage_ReadStats(int __formal, bool validate, bool silent);
void LiveStorage_InitCustomClassesNames();
void __cdecl LiveStorage_ReadStatsIfDirChanged();
void __cdecl LiveStorage_UploadStats();
void __cdecl LiveStorage_UploadStatsForController();
void __cdecl SV_ResetFileOp(dwFileOperationInfo *data);
dwFileOperationInfo *__cdecl SV_GetFreeFileOp();
void __cdecl SV_DumpFileOps();
void __cdecl SV_UseFileOp();
void __cdecl SV_FreeFileOp();
void __cdecl CL_GetXP_f();
void __cdecl LiveStorage_Init_Platform();
void __cdecl LiveStorage_FetchPlaylistsLocal(int controllerIndex);
void __cdecl LiveStorage_CheckOngoingSessionTasks_Platform();
bool __cdecl SV_GetStatFromBlob(char *buffer, const char *stat, int *outInt);
bool __cdecl SV_GetIntClientStatByGameMode(
                unsigned int clientNum,
                int *outInt,
                const char *gameMode,
                const char *statName);
void __cdecl SV_DWInitStats();
bool __cdecl SV_MakeClientLBRow(
                int clientNum,
                const LbPlayerStat *const stats,
                int type,
                int lbIndex,
                LeaderBoardRow<10> *const row,
                bool statWriteTypeAdd);
void __cdecl SV_CommitClientLeaderboardsSuccess();
void __cdecl SV_CommitClientLeaderboardsFailure();
void __cdecl SV_DoTimedStatsForClient(
                unsigned int clientnum,
                const char *gameModePrefix,
                LeaderBoardRow<10> *weeklyRow,
                int weeklylbIndex,
                LeaderBoardRow<10> *monthlyRow,
                int monthlylBIndex);
bool __cdecl SV_GetClientDDLStat(unsigned int clientnum, const char *stat, int *outInt);
int __cdecl LiveStorage_GetMatchesPlayed(const char *gameModePrefix, int resetPeriod);
char __cdecl SV_GetIntClientStatMatchDeltaByGameMode(
                int clientNum,
                int *outInt,
                const char *gameMode,
                const char *statName);
int __cdecl SV_GetTotalMatchesPlayedByGameModeForClient(unsigned int clientNum, const char *gameModePrefix);
bool __cdecl SV_ArePrestigeLBsEnabledForClient(client_t *client);
void __cdecl SV_SetGlobalLBsForClient(unsigned int clientNum);
void __cdecl SV_SetClientLBStat(unsigned int clientNum, int statId, int value);
bool __cdecl SV_MakeClientGlobalLBRow(
                int clientNum,
                const LbPlayerStat *const stats,
                int lbIndex,
                LeaderBoardRow<10> *const row);
void SV_CommitClientLeaderboards();
void __cdecl SV_SetClientStatsForRow(int clientNum, const char *gameModePrefix, bool delta);
void __cdecl SV_ReadClientFileSuccess(TaskRecord *task);
void __cdecl SV_WriteClientFileFailure(TaskRecord *task);
void __cdecl SV_WriteClientFileSuccess(TaskRecord *task);
void __cdecl SV_ReadDWFileDeferred(dwFileOperationInfo *fileInfo);
void __cdecl SV_WriteClientFileDeferred(client_t *client, dwFileOperationInfo *fileInfo);
void __cdecl SV_DWReadClientStats(client_t *client);
void __cdecl SV_DWReadClientGlobalStatsSuccess(const int controllerIndex, _QWORD *data);
char __cdecl SV_IsStatsBlobOK(char *data);
int __cdecl SV_DWReadClientGlobalStatsFailure(int controllerIndex, _QWORD *data);
void __cdecl SV_DWReadClientCAC(client_t *client);
void __cdecl SV_DWReadClientCACSuccess(const int controllerIndex, _QWORD *data);
int __cdecl SV_DWReadClientCACFailure(int controllerIndex, _QWORD *data);
void __cdecl SV_DWWriteClientStats(client_t *client);
void __cdecl SV_DWWriteClientGlobalStatsSuccess(int controllerIndex, unsigned __int8 **data);
void __cdecl LiveStorage_SendStatsBufferToClient(
                unsigned __int64 uid,
                unsigned __int8 *buffer,
                int buffersize,
                blobtype_t blobtype,
                bool sendOK);
char __cdecl SV_CACValidate_SetIntStat(unsigned __int8 *buffer, const char *stat, unsigned int value);
void __cdecl SV_PrestigeReset(char *liveStatsBuffer);
bool __cdecl SV_ValidateClientCAC(
                unsigned __int8 *oldcacblob,
                int oldcacblobsize,
                unsigned __int8 *newcacblob,
                int newcacblobsize,
                unsigned __int8 *globalblob,
                int globalblobsize,
                unsigned __int64 clientUID);
void __cdecl SV_CACValidate_EvaluateStatsBlobs(
                bool *oldcacok,
                bool *globalok,
                char *oldcacblob,
                char *globalblob,
                int oldcacsize,
                int globalsize);
void __cdecl SV_CACValidateSendClientMsgSuccess();
void __cdecl SV_CACValidateSendClientMsgFailure();
void __cdecl SV_CACValidateSendClientMsg(unsigned __int64 uid, unsigned int msg);
void __cdecl SV_CACValidateWriteCAC(unsigned __int64 client, unsigned __int8 *cacblob, unsigned int cacsize);
void __cdecl SV_CACValidateWriteCACSuccess(int controllerIndex, void *data);
void __cdecl SV_CACValidateWriteCACFailure(int controllerIndex, _QWORD *data);
void __cdecl SV_CACValidateWriteGlobal(unsigned __int64 client, unsigned __int8 *globalblob, unsigned int globalsize);
void __cdecl SV_CACValidateWriteGlobalSuccess(int controllerIndex, void *data);
void __cdecl SV_CACValidateWriteGlobalFailure(int controllerIndex, _QWORD *data);
TaskRecord *__cdecl SV_CACValidateReadCAC(unsigned __int64 client, unsigned __int8 *cacblob, unsigned int cacsize);
void __cdecl SV_CACValidateReadCACSuccess(int controllerIndex, void *data);
int __cdecl SV_CACValidateReadCACFailure(int controllerIndex, void *data);
TaskRecord *__cdecl SV_CACValidateReadGlobal(
                unsigned __int64 client,
                unsigned __int8 *globalblob,
                unsigned int globalblobsize);
void __cdecl SV_CACValidateReadGlobalSuccess(int controllerIndex, void *data);
int __cdecl SV_CACValidateReadGlobalFailure(int controllerIndex, _QWORD *data);
void __cdecl SV_CACValidateHandleRequest(
                unsigned __int64 clientID,
                unsigned __int8 *compressedcac,
                unsigned int cacsize);
void __cdecl Live_OnNewStatsFromServer(unsigned __int8 *compressedblob, unsigned int blobsize, blobtype_t blobtype);
char __cdecl Live_CACValidate_DispatchMessage(
                unsigned __int64 senderID,
                unsigned __int8 *message,
                unsigned int messageSize);
void __cdecl SV_SetPlaylistFetchedTime();
int __cdecl SV_GetPlaylistFetchedTime();
void __cdecl SV_SetShouldMapRotate(bool should);
bool __cdecl SV_ShouldMapRotate();
void __cdecl SV_FetchWADDeferred();
TaskRecord *__cdecl LiveStorage_ReadPlayerGlobalBlob();
void __cdecl LiveStorage_GetGlobalBlobSuccess(int controllerIndex, void *data);
int __cdecl LiveStorage_GetGlobalBlobFileNotFound(int controlleridx, void *data);
bool __cdecl LiveStorage_AreWeFetchingStats();
void __cdecl LiveStorage_BeginStatsFetch();
void __cdecl LiveStorage_FinishStatsFetch();
bool __cdecl LiveStorage_FirstTimeRunning();
void __cdecl LiveStorage_SetFirstTimeRunning(bool running);
void __cdecl LiveStorage_SaveRecentServersComplete(int controllerindex, void *data);
TaskRecord *__cdecl LiveStorage_SaveRecentServers(unsigned __int8 *buffer, int buffSize);
TaskRecord *__cdecl LiveStorage_ReadRecentServers(unsigned __int8 *buf, int bufsize);
void __cdecl LiveStorage_ReadRecentServersSuccess(int controllerIndex, void *data);
void __cdecl LiveStorage_DeleteGlobalStats(int controllerIndex);
void __cdecl LiveStorage_DeleteGlobalStatsSuccess();
void __cdecl LiveStorage_DeleteGlobalStatsNotFound();


extern sv_cacvalidate_state_t g_cacvalidateState;

extern const dvar_t *stats_backup;
extern const dvar_t *presell;
extern const dvar_t *sv_playlistFetchInterval;
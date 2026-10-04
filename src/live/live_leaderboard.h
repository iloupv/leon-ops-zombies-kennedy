#pragma once
#include <gfx_d3d/r_material.h>
#include <win32/win_tasks.h>
#include "live_storage.h"

enum LbTaskEnum : __int32
{                                       // XREF: ?LB_ReadHiddenLeaderboardComplete@@YA?AW4taskCompleteResults@@H@Z/r
                                        // ?Lb_ReadHiddenLeaderboardEntry@@YAXHPAV?$LeaderBoardRow@$09@@HW4LbTaskEnum@@@Z/r
    LB_TASK_READLB                 = 0x0,
    LB_TASK_READ_WEEKLY_HIDDEN_LB  = 0x1,
    LB_TASK_READ_PRESTIGE_WEEKLY_HIDDEN_LB = 0x2,
    LB_TASK_READ_MONTHLY_HIDDEN_LB = 0x3,
    LB_TASK_READ_PRESTIGE_MONTHLY_HIDDEN_LB = 0x4,
    LB_TASK_GETFRIENDS             = 0x5,
    LB_TASK_AM_UPLOADSTATS         = 0x6,
    LB_TASK_UPLOADSTATS            = 0x7,
    LB_TASK_UPLOADSTATS_WEEKLY_LB  = 0x8,
    LB_TASK_UPLOADSTATS_PRESTIGE_WEEKLY_LB = 0x9,
    LB_TASK_UPLOADSTATS_MONTHLY_LB = 0xA,
    LB_TASK_UPLOADSTATS_PRESTIGE_MONTHLY_LB = 0xB,
    LB_TASK_RESOLVEESCROW          = 0xC,
    LB_TASK_RETRIEVEESCROWBALANCE  = 0xD,
    LB_TASK_CLEARESCROW            = 0xE,
    LB_TASK_INCREMENTESCROW        = 0xF,
    LB_TASK_READLB_FOR_MATCHES_PLAYED = 0x10,
    LB_TASK_COUNT                  = 0x11,
};

enum LbStatEnum : __int32
{                                       // XREF: LbStructure/r
                                        // ?SV_MakeClientLBRow@@YA_NHQBULbPlayerStat@@HHQAV?$LeaderBoardRow@$09@@_N@Z/r ...
    LB_STAT_MATCHES_PLAYED    = 0x0,
    LB_STAT_SCORE             = 0x1,
    LB_STAT_TIME_PLAYED_TOTAL = 0x2,
    LB_STAT_SCORE_PER_MINUTE  = 0x3,
    LB_STAT_XP                = 0x4,
    LB_STAT_PRESTIGE_LEVEL    = 0x5,
    LB_STAT_KILLS             = 0x6,
    LB_STAT_DEATHS            = 0x7,
    LB_STAT_KDRATIO           = 0x8,
    LB_STAT_ASSISTS           = 0x9,
    LB_STAT_CAPTURES          = 0xA,
    LB_STAT_RETURNS           = 0xB,
    LB_STAT_DEFENDS           = 0xC,
    LB_STAT_PICKUPS           = 0xD,
    LB_STAT_PLANTS            = 0xE,
    LB_STAT_DEFUSES           = 0xF,
    LB_STAT_DESTRUCTIONS      = 0x10,
    LB_STAT_COUNT             = 0x11,
};

enum LbType : __int32
{                                       // XREF: Leaderboard/r LbGlob/r ...
    LB_TDM                  = 0x0,
    LB_GAMEMODE_START       = 0x0,
    LB_DM                   = 0x1,
    LB_CTF                  = 0x2,
    LB_DOM                  = 0x3,
    LB_SAB                  = 0x4,
    LB_SD                   = 0x5,
    LB_KOTH                 = 0x6,
    LB_DEM                  = 0x7,
    LB_BASIC_GAMEMODE_COUNT = 0x8,
    LB_HCTDM                = 0x8,
    LB_HARDCORE_TYPES_START = 0x8,
    LB_HCDM                 = 0x9,
    LB_HCCTF                = 0xA,
    LB_HCDOM                = 0xB,
    LB_HCSAB                = 0xC,
    LB_HCSD                 = 0xD,
    LB_HCKOTH               = 0xE,
    LB_HCDEM                = 0xF,
    LB_COUNT                = 0x10,
    LB_GAMEMODE_COUNT       = 0x10,
};

enum LbResetPeriod : __int32
{                                       // XREF: ?LB_GetLbIndex@@YAHW4LbType@@W4LbResetPeriod@@_N2@Z/r
                                        // ?LB_ReadHiddenLeaderboardComplete@@YA?AW4taskCompleteResults@@H@Z/r
    LB_RESETPERIOD_ALL_TIME = 0x0,
    LB_RESETPERIOD_WEEKLY   = 0x1,
    LB_RESETPERIOD_MONTHLY  = 0x2,
    LB_RESETPERIOD_COUNT    = 0x3,
};

enum GlobalLbStatEnum : __int32
{                                       // XREF: LbGlobalStructure/r
                                        // ?SV_MakeClientGlobalLBRow@@YA_NHQBULbPlayerStat@@HQAV?$LeaderBoardRow@$09@@@Z/r ...
    GLOBAL_LB_STAT_SCORE_PER_MINUTE = 0x0,
    GLOBAL_LB_STAT_KILLS            = 0x1,
    GLOBAL_LB_STAT_DEATHS           = 0x2,
    GLOBAL_LB_STAT_KDRATIO          = 0x3,
    GLOBAL_LB_STAT_ASSISTS          = 0x4,
    GLOBAL_LB_STAT_XP               = 0x5,
    GLOBAL_LB_STAT_PRESTIGE_LEVEL   = 0x6,
    GLOBAL_LB_STAT_TIMEPLAYED       = 0x7,
    GLOBAL_LB_STAT_WINS             = 0x8,
    GLOBAL_LB_STAT_LOSSES           = 0x9,
    GLOBAL_LB_STAT_WLRATIO          = 0xA,
    GLOBAL_LB_STAT_HEADSHOTS        = 0xB,
    GLOBAL_LB_STAT_ACCURACY         = 0xC,
    GLOBAL_LB_STAT_CODPOINTS        = 0xD,
    GLOBAL_LB_STAT_CURRENCYEARNED   = 0xE,
    GLOBAL_LB_STAT_CURRENCYSPENT    = 0xF,
    GLOBAL_LB_STAT_SCORE            = 0x10,
    GLOBAL_LB_STAT_WIN_STREAK       = 0x11,
    GLOBAL_LB_STAT_KILL_STREAK      = 0x12,
    GLOBAL_LB_STAT_COUNT            = 0x13,
};

struct LbStructure // sizeof=0x50
{                                       // XREF: LbLookup/r
                                        // ?LB_RegisterStructure@@YAXQAULbLookup@@ULbStructure@@H@Z/r ...
    int type;                           // XREF: LB_MakeRow(int,LbPlayerStat const * const,int,int,LeaderBoardRow<10> * const,bool)+B7/r
                                        // SV_MakeClientLBRow(int,LbPlayerStat const * const,int,int,LeaderBoardRow<10> * const,bool)+10E/r
    LbStatEnum columns[17];
    unsigned int numColums;
    unsigned int ratingCol;
};

struct LbPlayerStat // sizeof=0x58
{                                       // XREF: .data:LbPlayerStat * g_playerStats/r
                                        // LbGlob/r
    int values[19];                     // XREF: SV_DWInitStats(void)+61/w
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 userID;            // XREF: LB_UploadPlayerStats(int)+2E4/r
                                        // LB_UploadPlayerStats(int)+2EB/r ...
};

struct LbGlobalStructure // sizeof=0x58
{                                       // XREF: LbLookup/r
                                        // ?LB_RegisterGlobalStructure@@YAXQAULbLookup@@ULbGlobalStructure@@H@Z/r ...
    int lbIndex;                        // XREF: LB_MakeGlobalRow(int,LbPlayerStat const * const,int,LeaderBoardRow<10> * const)+A3/r
                                        // SV_MakeClientGlobalLBRow(int,LbPlayerStat const * const,int,LeaderBoardRow<10> * const)+B5/r
    GlobalLbStatEnum columns[19];
    unsigned int numColums;
    unsigned int ratingCol;
};

struct LbLookup // sizeof=0x668
{                                       // XREF: .data:LbLookup g_LbLookup/r
    unsigned int numLbs;                // XREF: LB_MakeRow(int,LbPlayerStat const * const,int,int,LeaderBoardRow<10> * const,bool):loc_968F03/r
                                        // SV_MakeClientLBRow(int,LbPlayerStat const * const,int,int,LeaderBoardRow<10> * const,bool):loc_988743/r
    LbStructure structures[16];         // XREF: LB_MakeRow(int,LbPlayerStat const * const,int,int,LeaderBoardRow<10> * const,bool)+B7/r
                                        // LB_MakeRow(int,LbPlayerStat const * const,int,int,LeaderBoardRow<10> * const,bool)+C8/o ...
    unsigned int numGlobalLbs;          // XREF: LB_MakeGlobalRow(int,LbPlayerStat const * const,int,LeaderBoardRow<10> * const):loc_9690D3/r
                                        // SV_MakeClientGlobalLBRow(int,LbPlayerStat const * const,int,LeaderBoardRow<10> * const)+75/r
    LbGlobalStructure globalStructures[4];
                                        // XREF: LB_MakeGlobalRow(int,LbPlayerStat const * const,int,LeaderBoardRow<10> * const)+A3/r
                                        // LB_MakeGlobalRow(int,LbPlayerStat const * const,int,LeaderBoardRow<10> * const)+B4/o ...
};

template <int SIZE>
struct LeaderBoardRow : bdStatsInfo // sizeof=0x98
{                                       // XREF: LeaderBoardResult<LeaderBoardRow<10>,100>/r
    int m_columns[SIZE];                  // XREF: LB_IncrementEscrow(void)+82/o

    inline void serialize(bdByteBuffer *buffer)
    {
#ifdef BO1_LIVE_STUBS
        unsigned int i; // [esp+4h] [ebp-4h]

        bdStatsInfo::serialize(buffer);
        for (i = 0; i < SIZE; ++i)
            bdByteBuffer::writeInt32(buffer, this->m_columns[i]);
#endif
    }
    inline bool deserialize(bdReference<bdByteBuffer> buffer)
    {
#ifdef BO1_LIVE_STUBS
        bdReference<bdCommonAddr> v3; // [esp-4h] [ebp-38h] BYREF
        bool v4; // [esp+3h] [ebp-31h]
        volatile int m_refCount; // [esp+4h] [ebp-30h]
        LeaderBoardRow<SIZE> *thisa; // [esp+8h] [ebp-2Ch]
        bdByteBuffer *v7; // [esp+18h] [ebp-1Ch]
        bdReference<bdCommonAddr> *v8; // [esp+1Ch] [ebp-18h]
        bdCommonAddr *m_ptr; // [esp+20h] [ebp-14h]
        bool v10; // [esp+27h] [ebp-Dh]
        unsigned int i; // [esp+2Ch] [ebp-8h]
        bool ok; // [esp+33h] [ebp-1h]

        thisa = this;
        v8 = &v3;
        v3.m_ptr = (bdCommonAddr *)buffer.m_ptr;

        if (buffer.m_ptr)
        {
            m_ptr = v8->m_ptr;
            InterlockedIncrement(&m_ptr->m_refCount);
            m_refCount = m_ptr->m_refCount;
        }

        v4 = bdStatsInfo::deserialize(v3);
        ok = v4;

        for (i = 0; ok && i < SIZE; ++i)
        {
            v7 = buffer.m_ptr;
            ok = bdByteBuffer::readInt32(buffer.m_ptr, &thisa->m_columns[i]);
        }

        v10 = ok;
        bdReference<bdRemoteTask>::~bdReference<bdRemoteTask>((bdReference<bdCommonAddr> *) & buffer);
        return v10;
#else
        return false;
#endif
    }
};

template <typename T, int SIZE>
struct LeaderBoardResult : bdStatsInfo //<LeaderBoardRow<10>,100> : bdStatsInfo // sizeof=0x3BD0
{                                       // XREF: Leaderboard/r
                                        // LbUserStats/r
    LeaderBoardRow<10> m_leaderBoardEntries[SIZE];// 100];
                                        // XREF: .rdata:off_D0DB08/o

    //LeaderBoardResult();
    //~LeaderBoardResult();
};

struct __declspec(align(8)) Leaderboard // sizeof=0x3CA0
{                                       // XREF: LbGlob/r
    LeaderBoardResult<LeaderBoardRow<10>,100> userStats;
                                        // XREF: .rdata:off_D0DB08/o
    LbType type;                        // XREF: LB_Init(void)+17C/w
    int offset;                         // XREF: LB_FeederCount(int)+36/r
                                        // LB_FeederItemText(int,int,int,Material * *)+C5/r ...
    int curIdx;                         // XREF: LB_FeederSelection(int)+14/w
                                        // LB_Init(void)+19E/w
    bool forceRefresh;                  // XREF: LB_ForceRefresh(void)+11/w
                                        // LB_Init(void)+1B1/w
    bool isUpdating;                    // XREF: LB_FeederItemText(int,int,int,Material * *)+57/r
    bool notSameFilter;
    bool notSameResetPeriod;
    bool notSamePrestigeMode;
    // padding byte
    // padding byte
    // padding byte
    int lastUpdate;                     // XREF: LB_Init(void)+140/w
    int filter;                         // XREF: LB_UpdateLeaderboard(int,LbType,int)+297/r
                                        // LB_Init(void)+153/w
    int lbTypeByResetPeriod;            // XREF: LB_Init(void)+166/w
    bool prestigeMode;
    // padding byte
    // padding byte
    // padding byte
    int numResults;
    int totalResults;                   // XREF: LB_FeederItemText(int,int,int,Material * *)+75/r
                                        // LB_FeederItemText(int,int,int,Material * *)+86/r
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    LeaderBoardRow<10> playerLbRow;
    int numMatchesPlayed;
    // padding byte
    // padding byte
    // padding byte
    // padding byte

    Leaderboard() = default;
    ~Leaderboard() = default;
};

struct LBSelectedPlayerInfo // sizeof=0x48
{                                       // XREF: .data:info_2/r
    unsigned __int64 playerXuid;        // XREF: UI_FeederDataExtended(int,itemDef_s *,char const *,char const * *,float *)+22B/r
                                        // UI_FeederDataExtended(int,itemDef_s *,char const *,char const * *,float *)+232/r
    char gamerTag[64];                  // XREF: UI_FeederDataExtended(int,itemDef_s *,char const *,char const * *,float *)+268/o
};

struct LbGlob // sizeof=0x3D528
{                                       // XREF: .data:LbGlob g_lbGlob/r
    overlappedTask tasks[32];           // XREF: LB_ReadHiddenLeaderboardComplete(int)+54/r
                                        // LB_ReadHiddenLeaderboardComplete(int)+8B/r ...
    LbPlayerStat playerStats[1];        // XREF: LB_ReadHiddenLeaderboardComplete(int)+367/o
                                        // LB_ClearPlayerStats(int)+37/o ...
    LeaderBoardRow<10> escrowData[1];   // XREF: LB_ResolveEscrowComplete(int)+191/o
    int escrowBalance[1];               // XREF: LB_RetrieveEscrowBalanceComplete(int)+1EB/w
                                        // LB_RetrieveEscrowBalanceComplete(int)+235/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    Leaderboard leaderboards[16];       // XREF: LB_ForceRefresh(void)+11/w
                                        // LB_UpdateLeaderboard(int,LbType,int)+F/o ...
    char feederText[32];                // XREF: LB_FeederItemText(int,int,int,Material * *):loc_96B5A2/w
                                        // LB_FeederItemText(int,int,int,Material * *)+141/o ...
    unsigned __int64 xuids[101];        // XREF: LB_GetStatsByXuids(int,Leaderboard *)+DA/o
                                        // LB_GetPlayerStats(int,Leaderboard *)+48/w ...
    unsigned int numXuids;              // XREF: LB_GetStatsByXuids(int,Leaderboard *)+6/r
                                        // LB_GetStatsByXuids(int,Leaderboard *):loc_967EA5/r ...
    LbType currentLbType;               // XREF: LB_Init(void):loc_96BD9D/w
    LeaderBoardRow<10> weeklyEntry[1];  // XREF: Lb_InitiateWeeklyAndMonthlyLbWrite(int,LbType)+28/o
    LeaderBoardRow<10> prestigeWeeklyEntry[1];
                                        // XREF: Lb_InitiateWeeklyAndMonthlyLbWrite(int,LbType)+A8/o
    LeaderBoardRow<10> monthlyEntry[1]; // XREF: Lb_InitiateWeeklyAndMonthlyLbWrite(int,LbType)+5F/o
    LeaderBoardRow<10> prestigeMonthlyEntry[1];
                                        // XREF: Lb_InitiateWeeklyAndMonthlyLbWrite(int,LbType)+DE/o

    LbGlob() = default;
    ~LbGlob() = default;
};

bool __cdecl LB_NeedToReadLeaderBoard();
void __cdecl LB_ForceRefresh();
void __cdecl LB_InitStructure(LbStructure *const structure, int type);
void __cdecl LB_RegisterStat(LbStructure *const structure, LbStatEnum statType, bool isRating);
void __cdecl LB_RegisterStructure(LbLookup *const lookup, LbStructure structure, int type);
void __cdecl LB_InitGlobalStructure(LbGlobalStructure *const globalStructures, int lbIndex);
void __cdecl LB_RegisterGlobalStat(
                LbGlobalStructure *const globalStructures,
                GlobalLbStatEnum globalStatType,
                bool isRating);
void __cdecl LB_RegisterGlobalStructure(LbLookup *const lookup, LbGlobalStructure globalStructures, int lbIndex);
LbType __cdecl LB_GetLbIndex(LbType type, LbResetPeriod resetPeriod, bool isPrestigeLb, bool isHiddenLb);
char __cdecl LB_GetNumMatchesPlayedForPlayerFromLb(Leaderboard *lb, int controllerIndex);
char __cdecl LB_GetByRank(Leaderboard *lb, int controllerIndex, unsigned int topRank);
char __cdecl LB_GetByPlayer(Leaderboard *lb, int localClientNum);
char __cdecl LB_GetStatsByXuids(int controllerIndex, Leaderboard *lb);
char __cdecl LB_GetPlayerStats(int controllerIndex, Leaderboard *lb);
char __cdecl LB_GetFriends(Leaderboard *lb, int localClientNum);
char __cdecl LB_CalcOffset(int offset, Leaderboard *lb);
LeaderBoardResult<LeaderBoardRow<10>,100> *__cdecl LB_UpdateLeaderboard(int localClientNum, LbType lbType, int index);
int __cdecl LB_FeederCount(int localClientNum);
void __cdecl LB_FeederSelection(int index);
void __cdecl LB_OnSelect(int localClientNum);
void __cdecl LB_GetSelectedPlayerInfo(int localClientNum, LBSelectedPlayerInfo *info);
void __cdecl LB_OverrideCursorPos(int viewmax, int *cursorPos, int *startPos);
int __cdecl LB_GetXUserCol(LbLookup *g_lbLookup, LeaderBoardRow<10> *row, int type, LbStatEnum colId);
void __cdecl LB_GetRankIconForXp(LeaderBoardRow<10> *row, LbType type, Material **handle);
unsigned int __cdecl LB_CalcPlayerIndex(int localControllerIndex, Leaderboard *lb);
void __cdecl LB_SortXUserStatsByRank(Leaderboard *lb);
//int __cdecl LB_CompareXUserStatsRowRanks(_QWORD *r1, _QWORD *r2);
bool __cdecl LB_MakeRow(
                int controllerIndex,
                const LbPlayerStat *const stats,
                int type,
                int lbIndex,
                LeaderBoardRow<10> *const row,
                bool statWriteTypeAdd);
bool __cdecl LB_MakeGlobalRow(
                int controllerIndex,
                const LbPlayerStat *const stats,
                int lbIndex,
                LeaderBoardRow<10> *const row);
void __cdecl LB_openDownloadingMenuCmd();
void __cdecl LB_ReportUser();
void __cdecl LB_IncrementEscrow();
char __cdecl LB_SetGametypeDvar();
taskCompleteResults __cdecl LB_ReadHiddenLeaderboardComplete(unsigned int slot);
void __cdecl LB_SetStatsForRow(int controllerIndex, const char *gameModePrefix, bool setMatchDelta);
taskCompleteResults __cdecl LB_ReadLeaderboardMatchesCountComplete(unsigned int slot);
taskCompleteResults __cdecl LB_ReadLeaderboardComplete(unsigned int slot);
void __cdecl LB_ClearPlayerStats(int localControllerIndex);
void __cdecl LB_SetStat(int localControllerIndex, int statId, int value);
void __cdecl LB_UpdateNeeded(int localControllerIndex);
void __cdecl LB_UploadPlayerGlobalStats(int localControllerIndex);
void __cdecl Lb_ReadHiddenLeaderboardEntry(
                int controllerIndex,
                LeaderBoardRow<10> *userStats,
                int lbIndex,
                LbTaskEnum task);
void __cdecl Lb_InitiateWeeklyAndMonthlyLbWrite(int controllerIndex, LbType type);
char __cdecl LB_UploadPlayerStats(int localControllerIndex);
taskCompleteResults __cdecl LB_UploadPlayerStatsComplete(unsigned int slot);
taskCompleteResults __cdecl LB_WriteStatsToLbComplete(unsigned int slot);
taskCompleteResults __cdecl LB_IncrementEscrowComplete(unsigned int slot);
taskCompleteResults __cdecl LB_ResolveEscrowComplete(unsigned int slot);
taskCompleteResults __cdecl LB_RetrieveEscrowBalanceComplete(unsigned int slot);
taskCompleteResults __cdecl LB_ClearEscrowComplete(int slot);
char *__cdecl LB_FeederItemText(int localClientNum, unsigned int index, int column, Material **material);
void __cdecl LB_ReadRowForUser(
                int controllerIndex,
                unsigned __int64 entityID,
                unsigned int leaderboardID,
                int taskType,
                LeaderBoardRow<10> *userStats);
void __cdecl LB_WriteRows(int controllerIndex, bdStatsInfo **statsInfo, unsigned int numStats, int taskType);
void __cdecl LB_InitAndRegisterStructure(LbStructure *const structure, int type);
void __cdecl LB_InitAndRegisterStructureForSpecialLeaderboards(
                LbGlobalStructure *const globalStructures,
                int lbIndex,
                int type);
void __cdecl LB_Init();
void __cdecl LB_CheckOngoingTasks();
void __cdecl LB_EndOngoingTasks();

extern const char *lbStatStrings[18];
extern const char *GlobalLbStatStrings[73];
extern LbLookup g_LbLookup;

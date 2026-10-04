#pragma once
#ifdef BO1_DW
#include <win32/win_tasks.h>
#include <live/live_win.h>

void __cdecl dwWriteStats(overlappedTask *const task, bdStatsInfo **statsInfo, unsigned int numStats);
taskCompleteResults __cdecl dwWriteStatsComplete(overlappedTask *const task);
void __cdecl dwReadStatsByUser(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                unsigned __int64 entityID,
                bdStatsInfo *statsInfo);
void __cdecl dwReadStatsByRank(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                unsigned int topRank,
                unsigned int maxResults,
                bdStatsInfo *statsInfo);
void __cdecl dwReadStatsByPivot(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                unsigned __int64 entityID,
                unsigned int maxResults,
                bdStatsInfo *statsInfo);
void __cdecl dwReadStatsForUsers(
                overlappedTask *const task,
                unsigned int leaderBoardID,
                const unsigned __int64 *entityIDs,
                unsigned int numEntityIDs,
                bdStatsInfo *statsInfo);
taskCompleteResults __cdecl dwReadStatsComplete(overlappedTask *const task, int *numResults, int *totalResults);
taskCompleteResults __cdecl dwFetchPerformanceValuesComplete(
                overlappedTask *const task,
                PlayerRank *playerRanks,
                int *numPlayerRanks);
taskCompleteResults __cdecl dwWritePerformanceStatsComplete(int controllerIndex, overlappedTask *const task);
#endif
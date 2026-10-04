#pragma once

#include <DemonWare/bdCore/bdReference/bdReferencable.h>
#include <live/live_storage.h>

struct overlappedTask // sizeof=0x24
{                                       // XREF: .data:overlappedIO/r
    bdReference<bdRemoteTask> overlappedIO;
                                        // XREF: dwRecordEventFormat(overlappedTask * const,uint,char const *,...)+5E/w
    bdRemoteTask::bdStatus finalStatus;
    bdLobbyErrorCode errorCode;
    int type;                           // XREF: MatchRecordEventComplete+51/r
                                        // LB_ReadHiddenLeaderboardComplete(int)+213/r ...
    bool active;                        // XREF: Live_CheckOngoingMatchRecordingTasks(void)+24/r
                                        // MatchRecordEventComplete+1A/r ...
    // padding byte
    // padding byte
    // padding byte
    int controllerIndex;                // XREF: MatchRecordEventComplete+86/r
                                        // LB_ReadLeaderboardComplete(int)+1E6/r ...
    void *data;                         // XREF: LB_ReadHiddenLeaderboardComplete(int)+8B/r
                                        // LB_ReadHiddenLeaderboardComplete(int)+1B7/r ...
    void *miscData;
    float startMS;
};

void __cdecl TaskManager_ClearOverlappedTasks(overlappedTask *overlappedTasks);
overlappedTask *__cdecl TaskManager_GetOpenTaskSlot(overlappedTask *overlappedTasks, int controllerIndex, int type);
void __cdecl TaskManager_ClearTask(overlappedTask *task);
char __cdecl TaskManager_AnyTaskInProgress(overlappedTask *overlappedTasks);
char __cdecl TaskManager_TaskIsInProgress(overlappedTask *overlappedTasks, int type);
void *__cdecl TaskManager_GetTaskData(overlappedTask *overlappedTask);

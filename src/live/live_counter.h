#pragma once
#include "live_storage.h"
#include <DemonWare/bdLobbyService.h>

struct bdCounterValue : bdTaskResult // sizeof=0x10
{                                       // XREF: .data:s_serverCounters/r
    unsigned int m_counterID;           // XREF: LiveCounter_UploadAllCounters(int)+57/r
    __int64 m_counterValue;             // XREF: LiveCounter_UploadAllCounters(int)+36/r
};

struct bdCounter // sizeof=0x4
{
    bdRemoteTaskManager *m_remoteTaskManager;
};

TaskRecord *__cdecl LiveCounter_IncrementCounters(
                const TaskDefinition *definition,
                int controllerIndex,
                bdCounterValue *const counterIncrements,
                unsigned int numEntries);
TaskRecord *__cdecl LiveCounter_GetCounterTotals(
                const TaskDefinition *definition,
                int controllerIndex,
                bdCounterValue *results,
                unsigned int numCounterIDs);
TaskRecord *__cdecl LiveCounter_UploadAllCounters(int controllerIndex);
void __cdecl LiveCounter_UploadAllCountersComplete();
void __cdecl LiveCounter_UploadAllCountersFailure(TaskRecord *task);
TaskRecord *__cdecl LiveCounter_DownloadAllCounters(int controllerIndex);
void __cdecl LiveCounter_DownloadAllCountersComplete();
void __cdecl LiveCounter_DownloadAllCountersFailure();
void __cdecl LiveCounter_IncrementCounterValueByName(const char *counterType, unsigned __int64 increment);
void __cdecl LiveCounter_IncrementCounterValue(int counterID, __int64 increment);
int __cdecl LiveCounter_GetCounterTotalValue(int counterID);
int __cdecl LiveCounter_CounterStringToID(const char *counterType);
void __cdecl LiveCounter_SetupCounters();
void __cdecl LiveCounter_Update(int controllerIndex);
void __cdecl LiveCounter_Init();
void __cdecl LiveCounter_IncrementCounter_f();
void __cdecl LiveCounter_Dump_f();

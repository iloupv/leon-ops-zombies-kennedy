#pragma once

#include <tl/jobqueue/jobqueue_all.h>

struct WorkerCmdConditional // sizeof=0x8
{
    void *address;
    unsigned int value;
};


void __cdecl Sys_WorkerCmdInit();
unsigned __int8 *__cdecl jqCommitMemory(jqBatch *batch, unsigned __int8 *input, unsigned int dataSize);
void __cdecl jqSafeFlush(jqBatchGroup *group, unsigned __int64 batchCount);
void *__cdecl jqLockData(jqBatch *batch);
void __cdecl jqUnlockData(jqBatch *batch);
void __cdecl Sys_AddWorkerCmdInternal(jqWorkerCmd *name, unsigned __int8 *data, WorkerCmdConditional *cond);
void __cdecl Sys_WaitWorkerCmdInternal(jqWorkerCmd *name);
void __cdecl Sys_AssistAndWaitWorkerCmdInternal(jqWorkerCmd *name);
bool __cdecl Sys_AssistAndWaitWorkerCmdInternalCallback(jqBatchGroup *context);
void __cdecl Sys_AssistSingle();
bool __cdecl Sys_AssistSingleCallback(unsigned int *context);
bool __cdecl Sys_AssistNeeded();
bool __cdecl R_FinishedFrontendWorkerCmds();
void __cdecl R_WaitFrontendWorkerCmds();
void __cdecl R_WaitWorkerCmds();

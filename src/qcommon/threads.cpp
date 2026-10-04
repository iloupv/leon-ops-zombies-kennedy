#include "threads.h"

#include <Windows.h>
#include <universal/q_shared.h>
#include "common.h"
#include <tl/jobqueue/jobqueue_all.h>
#include <live/live_win_common.h>
#include <gfx_d3d/r_cinematic.h>
#include <gfx_d3d/rb_resource.h>
#include <win32/win_common.h>

const char *s_threadNames[15] =
{
  "Main",
  "Backend",
  "Worker0",
  "Worker1",
  "Worker2",
  "Worker3",
  "Worker4",
  "Worker5",
  "Worker6",
  "Worker7",
  "Server",
  "occlusion",
  "TitleServer",
  "Database",
  "Stream"
};

unsigned int s_cpuCount;
DWORD threadId[15];
void *threadHandle[15];
void *g_threadValues[15][5];

thread_local void **g_dwTlsIndex;
thread_local unsigned int g_currentThreadId;

DWORD_PTR s_affinityMaskForProcess;
unsigned int s_affinityMaskForCpu[8];

void(__cdecl *threadFunc[15])(unsigned int);

void *demoStreamingReady;
unsigned int g_networkOverrideThread;
ThreadOwner g_discReadsOwner;

LONG smpData;
int g_databaseStopServer;
volatile unsigned int s_winThreadLock;

void *wakeServerEvent;
void *serverCompletedEvent;
void *allowServerNetworkEvent;
void *serverNetworkCompletedEvent;

void *sndInitializedEvent;
void *streamCompletedEvent;
void *streamEvent;
void *occlusionEvent;

void *wakeDatabaseEvent;
void *databaseCompletedEvent;
void *databaseCompletedEvent2;
void *resumedDatabaseEvent;

void *renderPausedEvent;
void *renderCompletedEvent;
void *resourcesFlushedEvent;
void *resourcesQueuedEvent;
void *rendererRunningEvent;
void *backendEvent[2];
void *updateSpotLightEffectEvent;
void *updateEffectsEvent;
void *d3dDeviceOKEvent;
void *d3dDeviceHardStartEvent;
void *d3dShutdownEvent;
void *d3dDeviceWinMessageEvent;
void *win32QuitEvent;
void *win32ScriptDebuggerDrawEvent;
void *rgRegisteredEvent;
void *renderEvent;
unsigned int __cdecl Sys_GetDefaultWorkerThreadsCount()
{
    unsigned int cpuCount; // [esp+0h] [ebp-4h]

    cpuCount = Sys_GetCpuCount();
    if ( cpuCount <= 4 )
        return 2;
    if ( cpuCount > 0xA )
        return 8;
    return cpuCount - 2;
}

unsigned int __cdecl Sys_GetCpuCount()
{
    return s_cpuCount;
}

void __cdecl Sys_InitMainThread()
{
    HANDLE process; // [esp+8h] [ebp-Ch]
    HANDLE pseudoHandle; // [esp+Ch] [ebp-8h]

    threadId[0] = Sys_GetCurrentThreadId();
    process = GetCurrentProcess();
    pseudoHandle = GetCurrentThread();
    DuplicateHandle(process, pseudoHandle, process, threadHandle, 0, 0, 2u);
    Win_InitThreads();
    SetThreadIdealProcessor(threadHandle[0], 0);
    g_dwTlsIndex = g_threadValues[0];
    Com_InitThreadData(0);
}

unsigned int __cdecl Sys_GetCurrentThreadId()
{
    if (!g_currentThreadId)
    {
        g_currentThreadId = GetCurrentThreadId();
    }
    return g_currentThreadId;
}

unsigned int Win_InitThreads()
{
    HANDLE CurrentProcess; // eax
    DWORD result; // eax
    DWORD cpuCount; // [esp+0h] [ebp-94h]
    DWORD systemAffinityMask; // [esp+4h] [ebp-90h] BYREF
    DWORD threadAffinityMask; // [esp+8h] [ebp-8Ch]
    DWORD affinityMaskBits[33]; // [esp+Ch] [ebp-88h]
    DWORD processAffinityMask; // [esp+90h] [ebp-4h] BYREF

    CurrentProcess = GetCurrentProcess();
    result = GetProcessAffinityMask(CurrentProcess, &processAffinityMask, &systemAffinityMask);
    s_affinityMaskForProcess = processAffinityMask;
    cpuCount = 0;
    for ( threadAffinityMask = 1; (processAffinityMask & -(int)threadAffinityMask) != 0; threadAffinityMask *= 2 )
    {
        if ( (processAffinityMask & threadAffinityMask) != 0 )
        {
            result = cpuCount;
            affinityMaskBits[cpuCount++] = threadAffinityMask;
            if ( cpuCount == 32 )
                break;
        }
        result = 2 * threadAffinityMask;
    }
    if ( cpuCount > 1 )
    {
        s_cpuCount = cpuCount;
        s_affinityMaskForCpu[0] = affinityMaskBits[0];
        result = affinityMaskBits[cpuCount - 1];
        s_affinityMaskForCpu[1] = result;
        if ( cpuCount != 2 )
        {
            if ( cpuCount == 3 )
            {
                s_affinityMaskForCpu[2] = affinityMaskBits[1];
            }
            else if ( cpuCount == 4 )
            {
                s_affinityMaskForCpu[2] = affinityMaskBits[1];
                result = affinityMaskBits[2];
                s_affinityMaskForCpu[3] = affinityMaskBits[2];
            }
            else
            {
                s_affinityMaskForCpu[0] = -1;
                s_affinityMaskForCpu[1] = -1;
                s_affinityMaskForCpu[2] = -1;
                s_affinityMaskForCpu[3] = -1;
                s_affinityMaskForCpu[4] = -1;
                s_affinityMaskForCpu[5] = -1;
                s_affinityMaskForCpu[6] = -1;
                s_affinityMaskForCpu[7] = -1;
                if ( s_cpuCount >= 8 )
                    s_cpuCount = 8;
            }
        }
    }
    else
    {
        s_cpuCount = 1;
        s_affinityMaskForCpu[0] = -1;
    }
    return result;
}

void __cdecl Sys_InitThread(int threadContext)
{
    g_dwTlsIndex = g_threadValues[threadContext];
    Com_InitThreadData(threadContext);
}

char __cdecl Sys_SpawnRenderThread(void (__cdecl *function)(unsigned int))
{
    Sys_CreateEvent(0, 0, &renderPausedEvent);
    Sys_CreateEvent(1, 1, &renderCompletedEvent);
    Sys_CreateEvent(1, 0, &resourcesFlushedEvent);
    Sys_CreateEvent(1, 0, &resourcesQueuedEvent);
    Sys_CreateEvent(1, 1, &rendererRunningEvent);
    Sys_CreateEvent(0, 0, &backendEvent[1]);
    Sys_CreateEvent(1, 0, &backendEvent[0]);
    Sys_CreateEvent(1, 1, &updateSpotLightEffectEvent);
    Sys_CreateEvent(1, 1, &updateEffectsEvent);
    Sys_CreateEvent(1, 1, &d3dDeviceOKEvent);
    Sys_CreateEvent(1, 0, &d3dDeviceHardStartEvent);
    Sys_CreateEvent(1, 0, &d3dShutdownEvent);
    Sys_CreateEvent(1, 1, &d3dDeviceWinMessageEvent);
    Sys_CreateEvent(1, 0, &win32QuitEvent);
    Sys_CreateEvent(1, 0, &win32ScriptDebuggerDrawEvent);
    Sys_CreateEvent(1, 0, &rgRegisteredEvent);
    Sys_CreateEvent(1, 0, &renderEvent);
    Sys_CreateThread(function, THREAD_CONTEXT_BACKEND);
    if ( !threadHandle[1] )
        return 0;
    SetThreadPriority(threadHandle[1], 1);
    if ( s_cpuCount == 1 )
        SetThreadIdealProcessor(threadHandle[1], 0);
    else
        SetThreadIdealProcessor(threadHandle[1], 1u);
    Sys_ResumeThread(THREAD_CONTEXT_BACKEND);
    return 1;
}

void __cdecl Sys_CreateEvent(bool manualReset, bool initialState, void **event)
{
    *event = CreateEventA(0, manualReset, initialState, 0);
}

void __cdecl Sys_CreateThread(void (__cdecl *function)(unsigned int), unsigned int threadContext)
{
    unsigned int LastError; // eax

    iassert(threadFunc[threadContext] == NULL);
    iassert(threadContext < THREAD_CONTEXT_COUNT);

    threadFunc[threadContext] = function;
    threadHandle[threadContext] = CreateThread(
        0,
        0,
        Sys_ThreadMain,
        (LPVOID)threadContext,
        CREATE_SUSPENDED,
        &threadId[threadContext]);

    iassert(threadHandle[threadContext] != NULL);

    if ( !threadHandle[threadContext] )
    {
        LastError = GetLastError();
        Com_Printf(1, "error %d while creating thread %d\n", LastError, threadContext);
    }
    SetThreadName(threadId[threadContext], s_threadNames[threadContext]);
}

void __cdecl SetThreadName(unsigned int dwThreadID, const char *szThreadName)
{
    tagTHREADNAME_INFO info; // [esp+10h] [ebp-28h] BYREF
    //CPPEH_RECORD ms_exc; // [esp+20h] [ebp-18h]

    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
    //ms_exc.registration.TryLevel = 0;
    //RaiseException(0x406D1388u, 0, 4u, &info.dwType);

    if ( dwThreadID == 0xFFFFFFFF || dwThreadID == GetCurrentThreadId() )
        PROF_THREADNAME(szThreadName);

    // this try/except needs to be here, otherwise it wont run without a debugger :D
    __try {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR *)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

DWORD WINAPI Sys_ThreadMain(LPVOID parameter)
{
    unsigned int threadContext = (unsigned int)parameter;

    bcassert(threadContext, THREAD_CONTEXT_COUNT);
    iassert(threadFunc[threadContext]);

    SetThreadName(0xFFFFFFFF, s_threadNames[threadContext]);

    Sys_InitThread(threadContext);
    threadFunc[threadContext](threadContext);

    return 0;
}

void __cdecl Sys_InitDemoStreamingEvent()
{
    Sys_CreateEvent(0, 0, &demoStreamingReady);
}

void __cdecl Sys_WaitForDemoStreamingEvent()
{
    Sys_WaitForSingleObject(&demoStreamingReady);
}

void __cdecl Sys_WaitForSingleObject(void **event)
{
    unsigned int result; // [esp+0h] [ebp-4h]

    result = WaitForSingleObject(*event, 0xFFFFFFFF);
    if ( result )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                        297,
                        0,
                        "%s\n\t(result) = %i",
                        "(result == ((((unsigned int)0x00000000L) ) + 0 ))",
                        result) )
            __debugbreak();
    }
}

void __cdecl Sys_SetDemoStreamingEvent()
{
    Sys_SetEvent(&demoStreamingReady);
}

void __cdecl Sys_SetEvent(void **event)
{
    SetEvent(*event);
}

void __cdecl Sys_InitServerEvents()
{
    // zombies: SP Sys_InitServerEvents (SP 0x004040F0) is the same four event calls and g_networkOverrideThread
    // (SP 0x0251C904) = 0. It runs on the server thread at its start (Com_ServerLoop SP 0x0087E2A0) and on the
    // main thread from SV_InitSnapshot (SP 0x005CC840, called by SV_SpawnServer and map_restart) while the
    // server thread keeps running its network loop (SP 0x0087E2A0: 0x0046BD70 wait allow, 0x00531EB0 take the
    // override, packets, 0x0057F500 release). In the SP retail exe the variable is written only (no assert
    // reads it), so clearing it under a running network loop is harmless there. BO1Zombies keeps the MP asserts:
    // a main-thread clear between the server thread's take and release fires threads.cpp:3131 in
    // Sys_SetServerNetworkCompletedEvent (seen at startup, while loading the map). So the clear waits, under
    // the override's own lock, until no thread holds the override - the same wait SV_AllowPackets does through
    // Sys_WaitServerNetworkCompleted - and the event calls stay SP's. Fixes both MP and zombies.
    Sys_EnterCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
    while ( g_networkOverrideThread && g_networkOverrideThread != Sys_GetCurrentThreadId() )
    {
        Sys_LeaveCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
        Sys_WaitForSingleObject(&serverNetworkCompletedEvent);
        Sys_EnterCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
    }
    Sys_ResetEvent(&wakeServerEvent);
    Sys_ResetEvent(&serverCompletedEvent);
    Sys_SetEvent(&allowServerNetworkEvent);
    Sys_SetEvent(&serverNetworkCompletedEvent);
    g_networkOverrideThread = 0;
    Sys_LeaveCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
}

void __cdecl Sys_ResetEvent(void **event)
{
    ResetEvent(*event);
}

char __cdecl Sys_SpawnServerThread(void (__cdecl *function)(unsigned int))
{
    Sys_CreateEvent(1, 0, &wakeServerEvent);
    Sys_CreateEvent(1, 0, &serverCompletedEvent);
    Sys_CreateEvent(1, 1, &allowServerNetworkEvent);
    Sys_CreateEvent(1, 1, &serverNetworkCompletedEvent);
    Sys_CreateThread(function, THREAD_CONTEXT_SERVER);
    if ( !threadHandle[10] )
        return 0;
    Sys_ResumeThread(THREAD_CONTEXT_SERVER);
    return 1;
}

char __cdecl Sys_SpawnDatabaseThread(void (__cdecl *function)(unsigned int))
{
    Sys_CreateEvent(0, 0, &wakeDatabaseEvent);
    Sys_CreateEvent(1, 1, &databaseCompletedEvent);
    Sys_CreateEvent(1, 1, &databaseCompletedEvent2);
    Sys_CreateEvent(1, 1, &resumedDatabaseEvent);
    Sys_CreateThread(function, THREAD_CONTEXT_DATABASE);
    if ( !threadHandle[13] )
        return 0;
    if ( s_cpuCount == 1 )
    {
        SetThreadIdealProcessor(threadHandle[13], 0);
    }
    else if ( s_cpuCount == 2 )
    {
        SetThreadIdealProcessor(threadHandle[13], 1u);
    }
    else
    {
        SetThreadIdealProcessor(threadHandle[13], 2u);
    }
    Sys_ResumeThread(THREAD_CONTEXT_DATABASE);
    return 1;
}

void __cdecl Sys_SuspendDiscReads(ThreadOwner owner)
{
    if ( owner == THREAD_OWNER_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    1325,
                    0,
                    "%s",
                    "owner != THREAD_OWNER_NONE") )
    {
        __debugbreak();
    }
    g_discReadsOwner = owner;
    Sys_ResetEvent(&resumedDatabaseEvent);
}

void __cdecl Sys_ResumeDiscReads(ThreadOwner owner)
{
    if ( owner == THREAD_OWNER_NONE
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    1343,
                    0,
                    "%s",
                    "owner != THREAD_OWNER_NONE") )
    {
        __debugbreak();
    }
    if ( g_discReadsOwner != owner
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    1344,
                    0,
                    "g_discReadsOwner == owner\n\t%i, %i",
                    g_discReadsOwner,
                    owner) )
    {
        __debugbreak();
    }
    g_discReadsOwner = THREAD_OWNER_NONE;
    Sys_SetEvent(&resumedDatabaseEvent);
}

bool __cdecl Sys_HaveSuspendedDiscReads(ThreadOwner owner)
{
    return g_discReadsOwner == owner;
}

void __cdecl Sys_WaitDiscReads()
{
    Sys_WaitForSingleObject(&resumedDatabaseEvent);
}

void __cdecl Sys_InitWorkerThreadContext()
{
    threadId[2] = jqWorkers[1].ThreadId;
    threadId[3] = jqWorkers[2].ThreadId;
}

void __cdecl Sys_ResumeThread(unsigned int threadContext)
{
    if ( !threadHandle[threadContext]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    1468,
                    0,
                    "%s",
                    "threadHandle[threadContext]") )
    {
        __debugbreak();
    }
    ResumeThread(threadHandle[threadContext]);
}

int __cdecl Sys_RendererSleep()
{
    return InterlockedExchange(&smpData, 0);
}

bool __cdecl Sys_RendererReady()
{
    return smpData != 0;
}

void __cdecl Sys_RenderCompleted()
{
    Sys_SetEvent(&renderCompletedEvent);
    Sys_SetWorkerCmdEvent();
}

void __cdecl Sys_FrontEndSleep()
{

    PROF_SCOPED("frontend sleep");
    Sys_WaitForSingleObject(&rendererRunningEvent);
}

int __cdecl Sys_WaitRenderer()
{
    PROF_SCOPED("Sys_WaitRenderer");

    if (IsDedicatedServer())
        return 0;

    if (!Sys_IsMainThread())
        return 0;

    while (Sys_QueryD3DDeviceOKEvent() && !Sys_WaitForSingleObjectTimeout(&renderCompletedEvent, 1u))
        ;

    return 0;
}

bool __cdecl Sys_WaitForSingleObjectTimeout(void **event, unsigned int msec)
{
    if ( msec == -1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp", 287, 0, "%s", "msec != INFINITE") )
    {
        __debugbreak();
    }
    return WaitForSingleObject(*event, msec) == 0;
}

void __cdecl Sys_WakeRenderer(void *data)
{
    Sys_ResetEvent(&renderCompletedEvent);
    smpData = (int)data;
    //PIXSetMarker(-1, "set smpData");
    Sys_SetEvent(&backendEvent[1]);
    Sys_SetWorkerCmdEvent();
}

void __cdecl Sys_NotifyRenderer()
{
    if ( !backendEvent[1]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    1641,
                    0,
                    "%s",
                    "Sys_IsEventInitialized( backendEvent[BACKEND_EVENT_GENERIC] )") )
    {
        __debugbreak();
    }
    Sys_SetEvent(&backendEvent[1]);
}

void __cdecl Sys_SleepServer()
{
    bool result; // [esp+8h] [ebp-4h]

    PROF_SCOPED("sleep server");
    result = Sys_WaitForSingleObjectTimeout(&wakeServerEvent, 0);
    if ( result )
        Sys_ResetEvent(&wakeServerEvent);
}

bool __cdecl Sys_WaitServer(unsigned int timeout)
{
    return Sys_WaitForSingleObjectTimeout(&serverCompletedEvent, timeout);
}

void __cdecl Sys_WakeServer()
{
    Sys_SetEvent(&wakeServerEvent);
}

void __cdecl Sys_ServerCompleted()
{
    Sys_SetEvent(&serverCompletedEvent);
}

bool __cdecl Sys_WaitStartServer(unsigned int timeout)
{
    bool start; // [esp+0h] [ebp-4h]

    start = Sys_WaitForSingleObjectTimeout(&wakeServerEvent, timeout);
    if ( start )
        Sys_ResetEvent(&serverCompletedEvent);
    if ( g_databaseStopServer )
        return 0;
    return start;
}

bool __cdecl Sys_IsServerThread()
{
    return Sys_GetCurrentThreadId() == threadId[10];
}

void __cdecl Sys_DatabaseCompleted()
{
    g_databaseStopServer = 1;
    if ( serverCompletedEvent )
        Sys_WaitForSingleObject(&serverCompletedEvent);
    Sys_SetEvent(&databaseCompletedEvent);
}

void __cdecl Sys_WaitStartDatabase()
{
    Sys_WaitForSingleObject(&wakeDatabaseEvent);
}

bool __cdecl Sys_IsDatabaseReady()
{
    bool result; // [esp+0h] [ebp-4h]

    result = Sys_WaitForSingleObjectTimeout(&databaseCompletedEvent, 0);
    if ( result )
        SocketRouter_EmergencyFrame();
    return result;
}

void __cdecl Sys_SyncDatabase()
{

    while ( !Sys_WaitForSingleObjectTimeout(&databaseCompletedEvent, 1u) )
    {
        R_Cinematic_ForceRelinquishIO();
        if ( Sys_IsRenderThread() )
            RB_Resource_Update(5);
        SocketRouter_EmergencyFrame();
    }
}

void __cdecl Sys_WakeDatabase()
{
    Sys_ResetEvent(&databaseCompletedEvent);
}

void __cdecl Sys_NotifyDatabase()
{
    Sys_SetEvent(&wakeDatabaseEvent);
}

void __cdecl Sys_DatabaseCompleted2()
{
    g_databaseStopServer = 0;
    Sys_SetEvent(&databaseCompletedEvent2);
}

bool __cdecl Sys_IsDatabaseReady2()
{
    bool result; // [esp+0h] [ebp-4h]

    result = Sys_WaitForSingleObjectTimeout(&databaseCompletedEvent2, 0);
    if ( result )
        SocketRouter_EmergencyFrame();
    return result;
}

void __cdecl Sys_WakeDatabase2()
{
    Sys_ResetEvent(&databaseCompletedEvent2);
}

void __cdecl Sys_StopRenderer()
{
    Sys_ResetEvent(&rendererRunningEvent);
    Sys_SetEvent(&renderPausedEvent);
}

void __cdecl Sys_StartRenderer()
{
    Sys_SetEvent(&rendererRunningEvent);
}

bool __cdecl Sys_IsRenderThread()
{
    return Sys_GetCurrentThreadId() == threadId[1];
}

bool __cdecl Sys_IsDatabaseThread()
{
    return Sys_GetCurrentThreadId() == threadId[13];
}

bool __cdecl Sys_IsMainThread()
{
    return Sys_GetCurrentThreadId() == threadId[0];
}

const char *__cdecl Sys_GetCurrentThreadName()
{
    return s_threadNames[Sys_GetThreadContext()];
}

int __cdecl Sys_GetThreadContext()
{
    int i; // [esp+0h] [ebp-8h]
    unsigned int currThread; // [esp+4h] [ebp-4h]

    currThread = Sys_GetCurrentThreadId();
    for ( i = 0; i < 15; ++i )
    {
        if ( threadId[i] == currThread )
            return i;
    }
    Com_Printf(1, "Current thread is not in thread table\n");
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp", 2277, 0, "%s", "0") )
        __debugbreak();
    return 15;
}

void __cdecl Sys_SetValue(int valueIndex, void *data)
{
    //*(unsigned int *)(*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 20) + 4 * valueIndex) = data;
    g_dwTlsIndex[valueIndex] = data;
}

void *__cdecl Sys_GetValue(int valueIndex)
{
    //return *(void **)(*(unsigned int *)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 20) + 4 * valueIndex);
    return g_dwTlsIndex[valueIndex];
}

void __cdecl Sys_SetWorkerCmdEvent()
{
    Sys_SetEvent(backendEvent);
}

bool __cdecl Sys_WaitBackendEvent(unsigned int msec)
{
    return Sys_WaitForSingleObjectTimeout(&backendEvent[1], msec);
}

void __cdecl Sys_SetUpdateSpotLightEffectEvent()
{
    Sys_SetEvent(&updateSpotLightEffectEvent);
}

void __cdecl Sys_ResetUpdateSpotLightEffectEvent()
{
    Sys_ResetEvent(&updateSpotLightEffectEvent);
}

void __cdecl Sys_WaitUpdateNonDependentEffectsCompleted()
{
    PROF_SCOPED("wait non-dependent fx");
    Sys_WaitForSingleObject(&updateEffectsEvent);
}

void __cdecl Sys_SetUpdateNonDependentEffectsEvent()
{
    Sys_SetEvent(&updateEffectsEvent);
}

void __cdecl Sys_ResetUpdateNonDependentEffectsEvent()
{
    Sys_ResetEvent(&updateEffectsEvent);
}

void __cdecl Sys_SuspendOtherThreads()
{
    unsigned int threadIndex; // [esp+0h] [ebp-8h]
    unsigned int currentThreadId; // [esp+4h] [ebp-4h]

    currentThreadId = Sys_GetCurrentThreadId();
    for ( threadIndex = 0; threadIndex < 0xF; ++threadIndex )
    {
        if ( threadHandle[threadIndex] && threadId[threadIndex] && threadId[threadIndex] != currentThreadId )
            SuspendThread(threadHandle[threadIndex]);
    }
}

void __cdecl Sys_SetWin32QuitEvent()
{
    Sys_SetEvent(&win32QuitEvent);
}

bool __cdecl Sys_QueryWin32QuitEvent()
{
    return Sys_WaitForSingleObjectTimeout(&win32QuitEvent, 0);
}

void __cdecl Sys_WaitD3DDeviceOKEvent()
{
    Sys_WaitForSingleObject(&d3dDeviceOKEvent);
}

void __cdecl Sys_ClearD3DDeviceOKEvent()
{
    Sys_ResetEvent(&d3dDeviceOKEvent);
}

void __cdecl Sys_SetD3DDeviceOKEvent()
{
    Sys_SetEvent(&d3dDeviceOKEvent);
}

bool __cdecl Sys_QueryD3DDeviceOKEvent()
{
    PROF_SCOPED("Sys_QueryD3DDeviceOKEvent"); // ADD
    return Sys_WaitForSingleObjectTimeout(&d3dDeviceOKEvent, 0);
}

void __cdecl Sys_WaitResourcesFlushedEvent()
{
    Sys_WaitForSingleObject(&resourcesFlushedEvent);
}

void __cdecl Sys_ClearResourcesFlushedEvent()
{
    Sys_ResetEvent(&resourcesFlushedEvent);
}

void __cdecl Sys_SetResourcesFlushedEvent()
{
    Sys_SetEvent(&resourcesFlushedEvent);
}

void __cdecl Sys_WaitResourcesQueuedEvent(unsigned int msec)
{
    Sys_WaitForSingleObjectTimeout(&resourcesQueuedEvent, msec);
}

void __cdecl Sys_ClearResourcesQueuedEvent()
{
    Sys_ResetEvent(&resourcesQueuedEvent);
}

void __cdecl Sys_SetResourcesQueuedEvent()
{
    Sys_SetEvent(&resourcesQueuedEvent);
}

void __cdecl Sys_SetD3DDeviceStartEvent()
{
    Sys_SetEvent(&d3dDeviceHardStartEvent);
}

void __cdecl Sys_ClearD3DDeviceStartEvent()
{
    Sys_ResetEvent(&d3dDeviceHardStartEvent);
}

void __cdecl Sys_WaitRGRegisteredEvent()
{
    Sys_WaitForSingleObject(&rgRegisteredEvent);
}

void __cdecl Sys_SetRGRegisteredEvent()
{
    Sys_SetEvent(&rgRegisteredEvent);
}

void __cdecl Sys_ClearRGRegisteredEvent()
{
    Sys_ResetEvent(&rgRegisteredEvent);
}

bool __cdecl Sys_QueryRGRegisteredEvent()
{
    return Sys_WaitForSingleObjectTimeout(&rgRegisteredEvent, 0);
}

bool __cdecl Sys_QueryRenderEvent()
{
    return Sys_WaitForSingleObjectTimeout(&renderEvent, 0);
}

void __cdecl Sys_SetD3DShutdownEvent()
{
    Sys_SetEvent(&d3dShutdownEvent);
}

void __cdecl Sys_ClearD3DShutdownEvent()
{
    Sys_ResetEvent(&d3dShutdownEvent);
}

bool __cdecl Sys_QueryD3DShutdownEvent()
{
    return Sys_WaitForSingleObjectTimeout(&d3dShutdownEvent, 0);
}

char __cdecl Sys_SpawnStreamThread(void (__cdecl *function)(unsigned int))
{
    Sys_CreateEvent(1, 0, &sndInitializedEvent);
    Sys_CreateEvent(1, 0, &streamCompletedEvent);
    Sys_CreateEvent(0, 0, &streamEvent);
    Sys_CreateThread(function, THREAD_CONTEXT_STREAM);
    if ( !threadHandle[14] )
        return 0;
    if ( s_cpuCount && s_cpuCount <= 2 )
        SetThreadIdealProcessor(threadHandle[14], 0);
    else
        SetThreadIdealProcessor(threadHandle[14], 2u);
    Sys_ResumeThread(THREAD_CONTEXT_STREAM);
    return 1;
}

int __cdecl Sys_StreamSleep()
{
    Sys_SetEvent(&streamCompletedEvent);
    Sys_WaitForSingleObject(&sndInitializedEvent);
    Sys_WaitForSingleObject(&streamEvent);
    Sys_ResetEvent(&streamCompletedEvent);
    return 0;
}

void __cdecl Sys_SetSndInitializedEvent()
{
    Sys_SetEvent(&sndInitializedEvent);
}

void __cdecl Sys_WakeStream()
{
    Sys_SetEvent(&streamEvent);
}

bool __cdecl Sys_IsStreamThread()
{
    return Sys_GetCurrentThreadId() == threadId[14];
}

void __cdecl Sys_SpawnOcclusion(void (__cdecl *function)(unsigned int))
{
    if ( threadHandle[THREAD_CONTEXT_OCCLUSION] )
    {
        Sys_ResumeThread(THREAD_CONTEXT_OCCLUSION);
    }
    else
    {
        Sys_CreateEvent(0, 0, &occlusionEvent);
        Sys_CreateThread(function, THREAD_CONTEXT_OCCLUSION);
        iassert(threadHandle[THREAD_CONTEXT_OCCLUSION]);
        if ( s_cpuCount && s_cpuCount <= 3 )
            SetThreadIdealProcessor(threadHandle[11], 0);
        else
            SetThreadIdealProcessor(threadHandle[11], 3u);
        SetThreadPriority(threadHandle[11], -1);
        Sys_ResumeThread(THREAD_CONTEXT_OCCLUSION);
    }
}

void __cdecl Sys_WaitForOcclusion()
{
    Sys_WaitForSingleObject(&occlusionEvent);
}

void __cdecl Sys_SetOcclusion()
{
    Sys_SetEvent(&occlusionEvent);
}

void __cdecl Sys_SetServerAllowNetworkEvent()
{
    Sys_SetEvent(&allowServerNetworkEvent);
}

void __cdecl Sys_ResetServerAllowNetworkEvent()
{
    Sys_ResetEvent(&allowServerNetworkEvent);
}

void __cdecl Sys_SetServerNetworkCompletedEvent()
{
    Sys_EnterCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
    if ( g_networkOverrideThread != Sys_GetCurrentThreadId()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    3131,
                    0,
                    "%s",
                    "g_networkOverrideThread == Sys_GetCurrentThreadId()") )
    {
        __debugbreak();
    }
    g_networkOverrideThread = 0;
    Sys_SetEvent(&serverNetworkCompletedEvent);
    Sys_LeaveCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
}

void __cdecl Sys_ResetServerNetworkCompletedEvent()
{
    Sys_EnterCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
    if ( g_networkOverrideThread
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    3148,
                    0,
                    "%s",
                    "!g_networkOverrideThread") )
    {
        __debugbreak();
    }
    g_networkOverrideThread = Sys_GetCurrentThreadId();
    Sys_ResetEvent(&serverNetworkCompletedEvent);
    Sys_LeaveCriticalSection(CRITSECT_NETTHREAD_OVERRIDE);
}

void __cdecl Sys_WaitAllowServerNetworkLoop()
{
    if ( !Sys_IsServerThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                    3166,
                    0,
                    "%s",
                    "Sys_IsServerThread()") )
    {
        __debugbreak();
    }
    Sys_WaitForSingleObject(&allowServerNetworkEvent);
}

void __cdecl Sys_WaitServerNetworkCompleted()
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp", 3173, 0, "%s", "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Sys_WaitForSingleObject(&serverNetworkCompletedEvent);
}

void __cdecl Win_LockThreadAffinity()
{
    if ( s_cpuCount != 1 && _InterlockedIncrement(&s_winThreadLock) == 1 )
    {
        if ( s_cpuCount < 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                        3204,
                        1,
                        "s_cpuCount >= 2\n\t%i, %i",
                        s_cpuCount,
                        2) )
        {
            __debugbreak();
        }
        if ( threadHandle[0] )
            SetThreadAffinityMask(threadHandle[0], s_affinityMaskForCpu[0]);
        if ( threadHandle[1] )
            SetThreadAffinityMask(threadHandle[1], s_affinityMaskForCpu[1]);
        if ( s_cpuCount < 3 )
        {
            if ( threadHandle[13] )
                SetThreadAffinityMask(threadHandle[13], s_affinityMaskForCpu[1]);
        }
        else if ( threadHandle[13] )
        {
            SetThreadAffinityMask(threadHandle[13], s_affinityMaskForCpu[2]);
        }
        if ( s_cpuCount >= 3 && threadHandle[2] )
            SetThreadAffinityMask(threadHandle[2], s_affinityMaskForCpu[2]);
        if ( s_cpuCount >= 4 )
        {
            if ( threadHandle[3] )
                SetThreadAffinityMask(threadHandle[3], s_affinityMaskForCpu[3]);
        }
    }
}

void __cdecl Win_UnlockThreadAffinity()
{
    if ( s_cpuCount != 1 && !_InterlockedDecrement(&s_winThreadLock) )
    {
        if ( s_cpuCount < 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\threads.cpp",
                        3251,
                        1,
                        "s_cpuCount >= 2\n\t%i, %i",
                        s_cpuCount,
                        2) )
        {
            __debugbreak();
        }
        if ( threadHandle[0] )
            SetThreadAffinityMask(threadHandle[0], s_affinityMaskForProcess);
        if ( threadHandle[1] )
            SetThreadAffinityMask(threadHandle[1], s_affinityMaskForProcess);
        if ( threadHandle[13] )
            SetThreadAffinityMask(threadHandle[13], s_affinityMaskForProcess);
        if ( threadHandle[2] )
            SetThreadAffinityMask(threadHandle[2], s_affinityMaskForProcess);
        if ( threadHandle[3] )
            SetThreadAffinityMask(threadHandle[3], s_affinityMaskForProcess);
    }
}


// mod (L64): bo1_mod_pcore. The engine sets no affinity/priority for the listen-server thread (Sys_SpawnServerThread), and
// Windows 10 (no hybrid-aware scheduling) often runs it on an E-core of a hybrid CPU (e.g. 8 P + 16 E cores).
// 1 = the server thread gets the P-cores' CPU sets (highest EfficiencyClass) and THREAD_PRIORITY_ABOVE_NORMAL;
// 2 = also the main, backend and job-queue worker threads (CPU sets only). No-op on a non-hybrid CPU or without the
// Windows 10 CPU set API (resolved at runtime). 0 after 1/2 restores what it changed. Called every frame (cheap).
typedef BOOL(WINAPI *SysGetSystemCpuSetInformation_t)(PSYSTEM_CPU_SET_INFORMATION, ULONG, PULONG, HANDLE, ULONG);
typedef BOOL(WINAPI *SysSetThreadSelectedCpuSets_t)(HANDLE, const ULONG *, ULONG);

static bool s_pcoreInit;
static SysSetThreadSelectedCpuSets_t s_setCpuSets;
static ULONG s_pcoreIds[64];
static ULONG s_pcoreCount;
static ULONG s_cpuSetTotal;
static signed char s_pcoreByLogical[64]; // group 0 logical processor -> 1 P-core / 0 slower class (hybrid CPUs only)

static void Sys_ModPCoreInit()
{
    if ( !s_pcoreInit )
    {
        s_pcoreInit = true;
        signed char classByLogical[64];
        memset(classByLogical, -1, sizeof(classByLogical));
        HMODULE kernel = GetModuleHandleA("kernel32.dll");
        SysGetSystemCpuSetInformation_t getInfo = kernel
            ? (SysGetSystemCpuSetInformation_t)GetProcAddress(kernel, "GetSystemCpuSetInformation") : NULL;
        s_setCpuSets = kernel ? (SysSetThreadSelectedCpuSets_t)GetProcAddress(kernel, "SetThreadSelectedCpuSets") : NULL;
        ULONG len = 0;
        if ( getInfo && s_setCpuSets )
            getInfo(NULL, 0, &len, GetCurrentProcess(), 0);
        unsigned char *buf = len ? (unsigned char *)malloc(len) : NULL;
        if ( buf && getInfo(( PSYSTEM_CPU_SET_INFORMATION )buf, len, &len, GetCurrentProcess(), 0) )
        {
            int maxClass = -1;
            for ( ULONG off = 0; off + sizeof(DWORD) * 2 <= len; off += (( PSYSTEM_CPU_SET_INFORMATION )(buf + off))->Size )
            {
                PSYSTEM_CPU_SET_INFORMATION info = ( PSYSTEM_CPU_SET_INFORMATION )(buf + off);
                if ( !info->Size )
                    break;
                if ( info->Type != CpuSetInformation )
                    continue;
                ++s_cpuSetTotal;
                if ( info->CpuSet.Group == 0 && info->CpuSet.LogicalProcessorIndex < ARRAYSIZE(classByLogical) )
                    classByLogical[info->CpuSet.LogicalProcessorIndex] = (signed char)info->CpuSet.EfficiencyClass;
                if ( info->CpuSet.EfficiencyClass > maxClass )
                {
                    maxClass = info->CpuSet.EfficiencyClass;
                    s_pcoreCount = 0;
                }
                if ( info->CpuSet.EfficiencyClass == maxClass && s_pcoreCount < ARRAYSIZE(s_pcoreIds) )
                    s_pcoreIds[s_pcoreCount++] = info->CpuSet.Id;
            }
            Com_Printf(16, "bo1_mod_pcore: %u of %u logical processors in the fastest efficiency class %d%s\n",
                s_pcoreCount, s_cpuSetTotal, maxClass, s_pcoreCount < s_cpuSetTotal ? "" : " (not hybrid: no-op)");
            for ( int i = 0; i < ARRAYSIZE(s_pcoreByLogical); ++i )
                s_pcoreByLogical[i] = classByLogical[i] < 0 || s_pcoreCount >= s_cpuSetTotal ? -1 : classByLogical[i] == maxClass;
        }
        else
        {
            Com_Printf(16, "bo1_mod_pcore: no CPU set API (needs Windows 10): no-op\n");
        }
        free(buf);
    }
}

// mod (L64): 1 = the calling thread runs on a P-core now, 0 = on a slower core, -1 = unknown / not a hybrid CPU (svprof's pcore_pct)
int __cdecl Sys_ModOnPCore()
{
    if ( !s_pcoreInit )
        return -1;
    DWORD cpu = GetCurrentProcessorNumber();
    return cpu < ARRAYSIZE(s_pcoreByLogical) ? s_pcoreByLogical[cpu] : -1;
}

void __cdecl Sys_ModApplyPCore(int mode)
{
    static int s_appliedMode;
    static HANDLE s_appliedHandles[5];
    static const int s_contexts[5] =
        { THREAD_CONTEXT_SERVER, THREAD_CONTEXT_MAIN, THREAD_CONTEXT_BACKEND, THREAD_CONTEXT_WORKER0, THREAD_CONTEXT_WORKER1 };
    HANDLE wanted[5];

    Sys_ModPCoreInit();
    if ( !s_setCpuSets || !s_pcoreCount || s_pcoreCount >= s_cpuSetTotal )
        mode = 0;
    bool same = mode == s_appliedMode;
    for ( int i = 0; i < 5; ++i )
    {
        wanted[i] = (mode >= 1 && i == 0) || mode >= 2 ? threadHandle[s_contexts[i]] : NULL;
        same = same && wanted[i] == s_appliedHandles[i];
    }
    if ( same )
        return;
    for ( int i = 0; i < 5; ++i )
    {
        HANDLE want = wanted[i];
        if ( s_appliedHandles[i] && s_appliedHandles[i] != want )
        {
            s_setCpuSets(s_appliedHandles[i], NULL, 0);
            if ( i == 0 )
                SetThreadPriority(s_appliedHandles[i], THREAD_PRIORITY_NORMAL);
        }
        if ( want && s_appliedHandles[i] != want )
        {
            s_setCpuSets(want, s_pcoreIds, s_pcoreCount);
            if ( i == 0 )
                SetThreadPriority(want, THREAD_PRIORITY_ABOVE_NORMAL);
        }
        s_appliedHandles[i] = want;
    }
    s_appliedMode = mode;
    Com_Printf(16, "bo1_mod_pcore %d: server thread %s, client threads %s\n", mode,
        s_appliedHandles[0] ? "on P-cores, above normal" : "unchanged", s_appliedHandles[1] ? "on P-cores" : "unchanged");
}

// mod (L64, orchestrator via L63): bo1_mod_priority. L63 saw the user's 0.1-6.6 s freezes as game threads ready but not scheduled
// while builds/other games loaded the PC. 1 = process priority class ABOVE_NORMAL + main thread above normal (the backend is
// already +1, Sys_SpawnRenderThread); 2 = HIGH class. 0 after 1/2 restores the class the process started with (headless.ps1
// starts games BELOW_NORMAL). Called every frame next to Sys_ModApplyPCore; returns at once unless the mode changed.
void __cdecl Sys_ModApplyPriority(int mode)
{
    static int s_appliedPriority;
    static DWORD s_startClass;
    if ( mode == s_appliedPriority )
        return;
    if ( !s_startClass )
        s_startClass = GetPriorityClass(GetCurrentProcess());
    SetPriorityClass(GetCurrentProcess(), mode == 2 ? HIGH_PRIORITY_CLASS : mode == 1 ? ABOVE_NORMAL_PRIORITY_CLASS : s_startClass);
    if ( threadHandle[THREAD_CONTEXT_MAIN] )
        SetThreadPriority(threadHandle[THREAD_CONTEXT_MAIN], mode ? THREAD_PRIORITY_ABOVE_NORMAL : THREAD_PRIORITY_NORMAL);
    s_appliedPriority = mode;
    Com_Printf(16, "bo1_mod_priority %d: process class 0x%x (started 0x%x), main thread %s\n", mode,
        GetPriorityClass(GetCurrentProcess()), s_startClass, mode ? "above normal" : "normal");
}
